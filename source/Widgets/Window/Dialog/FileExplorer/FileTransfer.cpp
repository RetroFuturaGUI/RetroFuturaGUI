#include "FileTransfer.hpp"
#include "FileSystem.hpp"
#include <cerrno>
#include <format>
#include <fstream>
#include <memory>

RetroFuturaGUI::FileTransfer::~FileTransfer()
{
    Stop(); //join worker before the rest goes
}

bool RetroFuturaGUI::FileTransfer::Start(Request&& request)
{
    {
        std::scoped_lock lock { _mutex };

        if(_isBusy)
            return false;

        _isBusy = true;
        _progress = Progress { ._Operation = request._Operation, ._DoneCount = 0, ._TotalCount = request._Sources.size(), ._Current = {} };
        _clash.reset();
        _clashAnswer.reset();
        _outcome.reset();
    }

    //The worker before it left its outcome as its last act, so it is done or about to return
    if(_worker.joinable())
        _worker.join();

    _worker = std::jthread([this, request = std::move(request)](std::stop_token stop) mutable { run(std::move(request), stop); });
    return true;
}

bool RetroFuturaGUI::FileTransfer::IsBusy() const
{
    std::scoped_lock lock { _mutex };
    return _isBusy;
}

void RetroFuturaGUI::FileTransfer::Stop()
{
    _worker.request_stop(); //also wakes a worker waiting for a clash answer
}

RetroFuturaGUI::FileTransfer::Progress RetroFuturaGUI::FileTransfer::GetProgress() const
{
    std::scoped_lock lock { _mutex };
    return _progress;
}

std::optional<RetroFuturaGUI::FileTransfer::Clash> RetroFuturaGUI::FileTransfer::GetClash() const
{
    std::scoped_lock lock { _mutex };
    return _clash;
}

void RetroFuturaGUI::FileTransfer::AnswerClash(const ClashAnswer answer, const ClashScope scope)
{
    {
        std::scoped_lock lock { _mutex };

        if(!_clash)
            return;

        _clashAnswer = std::pair { answer, scope };
    }

    _clashAnswered.notify_all();
}

std::optional<RetroFuturaGUI::FileTransfer::Outcome> RetroFuturaGUI::FileTransfer::TakeOutcome()
{
    std::scoped_lock lock { _mutex };

    if(!_outcome)
        return std::nullopt;

    std::optional<Outcome> outcome { std::move(_outcome) };
    _outcome.reset();
    _isBusy = false;
    return outcome;
}

bool RetroFuturaGUI::FileTransfer::IsInside(const std::filesystem::path& path, const std::filesystem::path& folder)
{
    //By the file system's identity, so neither case nor spelling fools it
    std::error_code error {};

    for(std::filesystem::path ancestor { path }; !ancestor.empty(); ancestor = ancestor.parent_path())
    {
        if(std::filesystem::equivalent(ancestor, folder, error))
            return true;

        if(!ancestor.has_relative_path())
            break; //a root is its own parent
    }

    return false;
}

void RetroFuturaGUI::FileTransfer::run(Request&& request, std::stop_token stop)
{
    Outcome outcome { ._Operation = request._Operation, ._TotalCount = request._Sources.size(), ._Done = {}, ._NotTrashed = {}, ._Skipped = {}, ._FailedPath = {}, ._Error = {}, ._FailedCount = 0, ._IsStopped = false };
    std::optional<ClashAnswer> standingAnswer {}; //given for the remaining clashes

    for(uSize index = 0; index < request._Sources.size(); ++index)
    {
        if(stop.stop_requested())
        {
            outcome._IsStopped = true;
            break;
        }

        const std::filesystem::path& source { request._Sources[index] };
        setProgress(index, source);
        std::error_code error {};

        switch(request._Operation)
        {
            case Operation::MoveToTrash:
            {
                const PlatformBridge::FileSystem::TrashResult result { PlatformBridge::FileSystem::MoveToTrash(source, error) };

                if(PlatformBridge::FileSystem::TrashResult::Moved == result)
                    outcome._Done.push_back(DoneItem { ._From = source, ._To = {} });
                else if(PlatformBridge::FileSystem::TrashResult::NotTrashable == result)
                    outcome._NotTrashed.push_back(source);
                else
                    noteFailure(outcome, source, error);
            }
            break;
            case Operation::DeletePermanently:
                if(deleteTree(source, stop, error))
                    outcome._Done.push_back(DoneItem { ._From = source, ._To = {} });
                else if(stop.stop_requested())
                    outcome._IsStopped = true;
                else
                    noteFailure(outcome, source, error);
            break;
            case Operation::RestoreFromTrash:
            {
                std::filesystem::path restored {};

                if(PlatformBridge::FileSystem::RestoreFromTrash(source, restored, error))
                    outcome._Done.push_back(DoneItem { ._From = source, ._To = restored });
                else
                    noteFailure(outcome, source, error);
            }
            break;
            default:
                transferItem(request, index, outcome, standingAnswer, stop);
            break;
        }
    }

    std::scoped_lock lock { _mutex };
    _progress._DoneCount = request._Sources.size();
    _clash.reset();
    _outcome = std::move(outcome);
}

void RetroFuturaGUI::FileTransfer::transferItem(const Request& request, const uSize index, Outcome& outcome, std::optional<ClashAnswer>& standingAnswer, std::stop_token stop)
{
    const std::filesystem::path& source { request._Sources[index] };
    std::error_code error {};
    const std::filesystem::file_status status { std::filesystem::symlink_status(source, error) };

    if(error)
    {
        noteFailure(outcome, source, error); //gone since it was picked
        return;
    }

    std::filesystem::path
        target { index < request._Targets.size() ? request._Targets[index] : request._Destination / source.filename() },
        aside {}; //what a Replace moves out of the way until the item is in
    const std::filesystem::path destination { target.parent_path() };
    std::filesystem::create_directories(destination, error); //moved back, the folder it came from may have gone

    const bool
        isMove { Operation::Move == request._Operation },
        isFolder { std::filesystem::is_directory(status) },
        isSameFolder { std::filesystem::equivalent(source.parent_path(), destination, error) };

    //It would copy itself into itself without end
    if(isFolder && IsInside(destination, source))
    {
        noteFailure(outcome, source, std::make_error_code(std::errc::invalid_argument));
        return;
    }

    if(isMove && source.filename() == target.filename() && isSameFolder)
        return; //a cut pasted where it is stays

    if(std::filesystem::exists(std::filesystem::symlink_status(target, error)))
    {
        ClashAnswer answer { ClashAnswer::KeepBoth };

        //A copy into its own folder is a second one, whatever the rule
        if(!isSameFolder && ClashRule::Ask == request._ClashRule)
        {
            if(standingAnswer)
            {
                answer = *standingAnswer;
            }
            else
            {
                const std::optional<std::pair<ClashAnswer, ClashScope>> reply { askAboutClash(source, target, stop) };

                if(!reply)
                {
                    outcome._IsStopped = true;
                    return;
                }

                answer = reply->first;

                if(ClashScope::RemainingItems == reply->second)
                    standingAnswer = answer;
            }
        }

        if(ClashAnswer::Skip == answer)
        {
            outcome._Skipped.push_back(source);
            return;
        }

        if(ClashAnswer::KeepBoth == answer)
        {
            target = PlatformBridge::FileSystem::FreeName(target, isFolder);
        }
        else
        {
            if(std::filesystem::equivalent(source, target, error))
                return; //replacing an item with itself leaves it

            aside = target;
            aside += ".replaced";
            aside = PlatformBridge::FileSystem::FreeName(aside, true);
            std::filesystem::rename(target, aside, error);

            if(error)
            {
                noteFailure(outcome, source, error);
                return;
            }
        }
    }

    const bool isDone { isMove ? moveTree(source, target, stop, error) : copyTree(source, target, stop, error) };

    //The replaced item goes once the new one is in, and comes back when it didn't make it
    if(!aside.empty())
    {
        std::error_code asideError {};

        if(isDone)
        {
            deleteTree(aside, {}, asideError);
        }
        else
        {
            deleteTree(target, {}, asideError);
            std::filesystem::rename(aside, target, asideError);
        }
    }

    if(isDone)
        outcome._Done.push_back(DoneItem { ._From = source, ._To = target });
    else if(stop.stop_requested())
        outcome._IsStopped = true;
    else
        noteFailure(outcome, source, error);
}

std::optional<std::pair<RetroFuturaGUI::FileTransfer::ClashAnswer, RetroFuturaGUI::FileTransfer::ClashScope>> RetroFuturaGUI::FileTransfer::askAboutClash(const std::filesystem::path& source, const std::filesystem::path& target, std::stop_token stop)
{
    std::unique_lock lock { _mutex };
    _clash = Clash { ._Source = source, ._Target = target };
    _clashAnswer.reset();

    const bool isAnswered { _clashAnswered.wait(lock, stop, [this]() { return _clashAnswer.has_value(); }) };
    _clash.reset();

    if(!isAnswered)
        return std::nullopt; //stopped while it waited

    const std::optional<std::pair<ClashAnswer, ClashScope>> answer { _clashAnswer };
    _clashAnswer.reset();
    return answer;
}

void RetroFuturaGUI::FileTransfer::setProgress(const uSize doneCount, const std::filesystem::path& current)
{
    std::scoped_lock lock { _mutex };
    _progress._DoneCount = doneCount;
    _progress._Current = current;
}

void RetroFuturaGUI::FileTransfer::noteFailure(Outcome& outcome, const std::filesystem::path& path, const std::error_code& error)
{
    if(0 == outcome._FailedCount)
    {
        outcome._FailedPath = path;
        outcome._Error = error;
    }

    ++outcome._FailedCount;
}

bool RetroFuturaGUI::FileTransfer::copyTree(const std::filesystem::path& source, const std::filesystem::path& target, std::stop_token stop, std::error_code& errorOut)
{
    const std::filesystem::file_status status { std::filesystem::symlink_status(source, errorOut) };

    if(errorOut)
        return false;

    //A link is copied as a link, never what it points to
    if(std::filesystem::is_symlink(status))
    {
        std::filesystem::copy_symlink(source, target, errorOut);
        return !errorOut;
    }

    if(std::filesystem::is_regular_file(status))
        return copyFile(source, target, stop, errorOut);

    if(!std::filesystem::is_directory(status))
        return true; //devices, sockets and junctions aren't copied

    std::filesystem::create_directory(target, source, errorOut); //with the original's attributes

    if(errorOut)
        return false;

    for(std::filesystem::directory_iterator iterator { source, errorOut }, end {}; !errorOut && iterator != end; iterator.increment(errorOut))
    {
        if(stop.stop_requested())
            return false;

        if(!copyTree(iterator->path(), target / iterator->path().filename(), stop, errorOut))
            return false;
    }

    return !errorOut;
}

bool RetroFuturaGUI::FileTransfer::copyFile(const std::filesystem::path& source, const std::filesystem::path& target, std::stop_token stop, std::error_code& errorOut)
{
    errorOut.clear();
    std::ifstream input { source, std::ios::binary };

    if(!input)
    {
        errorOut = std::error_code(errno, std::generic_category());
        return false;
    }

    std::ofstream output { target, std::ios::binary | std::ios::trunc };

    if(!output)
    {
        errorOut = std::error_code(errno, std::generic_category());
        return false;
    }

    const std::unique_ptr<char[]> buffer { std::make_unique_for_overwrite<char[]>(_chunkSize) };

    while(input && !stop.stop_requested())
    {
        input.read(buffer.get(), static_cast<std::streamsize>(_chunkSize));
        const std::streamsize readCount { input.gcount() };

        if(0 < readCount)
            output.write(buffer.get(), readCount);

        if(!output)
        {
            errorOut = std::error_code(errno, std::generic_category()); //a full disk, mostly
            break;
        }
    }

    const bool isRead { input.eof() && !input.bad() };
    output.close();

    if(!isRead || output.fail() || stop.stop_requested())
    {
        if(!errorOut && !stop.stop_requested())
            errorOut = std::make_error_code(std::errc::io_error);

        std::error_code removeError {};
        std::filesystem::remove(target, removeError); //the file copied halfway
        return false;
    }

    std::error_code timeError {};
    std::filesystem::last_write_time(target, std::filesystem::last_write_time(source, timeError), timeError);
    std::filesystem::permissions(target, std::filesystem::status(source, timeError).permissions(), timeError); //read-only stays read-only
    return true;
}

bool RetroFuturaGUI::FileTransfer::deleteTree(const std::filesystem::path& path, std::stop_token stop, std::error_code& errorOut)
{
    const std::filesystem::file_status status { std::filesystem::symlink_status(path, errorOut) };

    if(errorOut)
        return false;

    //A link goes alone, never what it points to
    if(std::filesystem::is_directory(status))
    {
        for(std::filesystem::directory_iterator iterator { path, errorOut }, end {}; !errorOut && iterator != end; iterator.increment(errorOut))
        {
            if(stop.stop_requested())
                return false;

            if(!deleteTree(iterator->path(), stop, errorOut))
                return false;
        }

        if(errorOut)
            return false;
    }

    std::filesystem::remove(path, errorOut);
    return !errorOut;
}

bool RetroFuturaGUI::FileTransfer::moveTree(const std::filesystem::path& source, const std::filesystem::path& target, std::stop_token stop, std::error_code& errorOut)
{
    std::filesystem::rename(source, target, errorOut);

    if(!errorOut)
        return true;

    //On the same volume, the rename really failed. The error can't tell us: Windows reports a folder moved to another volume as "access denied", so the volumes are compared instead
    if(PlatformBridge::FileSystem::IsSameVolume(source, target.parent_path()))
        return false;

    //To another volume: copy everything first, then delete the original, so a stop or a failure leaves the original intact
    errorOut.clear();

    if(!copyTree(source, target, stop, errorOut))
    {
        std::error_code cleanupError {};
        deleteTree(target, {}, cleanupError);
        return false;
    }

    return deleteTree(source, {}, errorOut); //can't be stopped: stopping halfway would leave the files split between both places
}