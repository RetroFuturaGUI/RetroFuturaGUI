#pragma once
#include "config.hpp"
#include <chrono>
#include <condition_variable>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <stop_token>
#include <thread>
#include <utility>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief One worker thread running jobs in the order they were posted. A job's result comes back to the thread that calls
    ///        Drain, once per frame, so the slot handling it can touch widgets and GL objects. Destroying it sets the running
    ///        job's stop token and drops everything not yet run or drained. Whether it waits for the running job is chosen at construction.
    class BackgroundWork
    {
    public:
        /// @brief What destroying a BackgroundWork does with the running job.
        enum class WhenDestroyed : u32
        {
            WaitForJob, //waits for it to return, so a job may use what its owner holds
            LetJobFinish, //lets it finish on its own and drops its result, so a job stuck on a slow drive holds nothing up. Its
                          //work must use only what it was given; at program exit it is cut off wherever it is, unless WaitForLetGoJobs waits for it.
            CancelJob     //as LetJobFinish, and also ends the file system call the job may be blocked in (PlatformBridge::FileSystem::CancelThreadIo),
                          //so a listing of a drive that stopped answering ends early. Only for jobs whose blocked call may fail without harm
        };

        explicit BackgroundWork(const WhenDestroyed whenDestroyed);
        BackgroundWork() = delete;
        BackgroundWork(const BackgroundWork&) = delete;
        BackgroundWork(BackgroundWork&&) = delete;
        auto operator =(const BackgroundWork&) = delete;
        auto operator =(BackgroundWork&&) = delete;
        ~BackgroundWork();

        /// @brief Queues work for the worker thread, and the slot that takes its result on the thread calling Drain.
        /// @param work Called as work(std::stop_token) on the worker; it returns the result. A long job checks the token, which is set while the BackgroundWork is destroyed
        /// @param done Called as done(Result&&) from Drain. It mustn't destroy this BackgroundWork.
        template<typename Work, typename Done> void Post(Work&& work, Done&& done)
        {
            queue([work = std::forward<Work>(work), done = std::forward<Done>(done)](std::stop_token stop) mutable -> Finish
            {
                auto result { work(stop) };

                return [done = std::move(done), result = std::move(result)]() mutable { done(std::move(result)); };
            });
        }

        /// @brief Runs the slots of the jobs that finished since the last call in the order the jobs were posted.
        void Drain();

        /// @brief Drops the jobs that haven't started and the results not yet drained. The running job still finishes, and its
        ///        result still arrives; a caller that no longer wants it tells it apart, by a generation number for instance.
        void ClearPending();

        /// @brief Returns whether a job is queued, running, or a result waits for Drain.
        bool IsBusy() const;

        /// @brief Waits until the jobs let go to finish on their own have ended, or the limit passed. Call it at program exit, after
        ///        the last BackgroundWork that lets its job go is destroyed and before main returns, so no job is still running while
        ///        the program's static objects are destroyed. A job checking its stop token ends within moments; one blocked in the
        ///        system for longer is cut off at exit as before
        /// @return Whether all of them ended in time
        static bool WaitForLetGoJobs(const std::chrono::milliseconds limit);

    private:
        using Finish = std::move_only_function<void()>; //a job's slot bound to its result, run by Drain
        using Job = std::move_only_function<Finish(std::stop_token)>;

        /// @brief What the worker thread shares with its BackgroundWork. Owned by both, so a thread let go to finish its job still has it once the BackgroundWork is gone.
        struct State
        {
            mutable std::mutex _Mutex {};
            std::condition_variable_any _Wake {};
            std::deque<Job> _Jobs {};
            std::vector<Finish> _Finished {};
            bool
                _IsRunningJob { false },
                _IsLetGo { false },   //its BackgroundWork is gone and LetGoJobs counts its thread
                _HasEnded { false };  //its thread returned
        };

        /// @brief The threads let go to finish on their own that haven't ended. Made once and never destroyed, so a thread ending
        ///        while the program's static objects are destroyed can still reach it.
        struct LetGoJobs
        {
            std::mutex _Mutex {};
            std::condition_variable _Ended {};
            uSize _RunningCount { 0 };
        };

        static LetGoJobs& letGoJobs();

        /// @brief The worker thread's last act: marks its state ended and, when it was let go, takes it off LetGoJobs
        static void endThread(State& state);

        void queue(Job&& job);

        /// @brief The worker thread: runs one job after the other until a stop is requested.
        static void run(State& state, std::stop_token stop);

    //Logic
        std::shared_ptr<State> _state { std::make_shared<State>() };
        WhenDestroyed _whenDestroyed { WhenDestroyed::WaitForJob };
        std::jthread _worker {}; //last, so it is stopped before the members it reads are destroyed
    };
}