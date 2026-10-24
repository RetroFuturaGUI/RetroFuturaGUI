#pragma once
#include "config.hpp"
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
            LetJobFinish //lets it finish on its own and drops its result, so a job stuck on a slow drive holds nothing up. Its
                         //work must use only what it was given; at program exit it is cut off wherever it is
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
            bool _IsRunningJob { false };
        };

        void queue(Job&& job);

        /// @brief The worker thread: runs one job after the other until a stop is requested.
        static void run(State& state, std::stop_token stop);

    //Logic
        std::shared_ptr<State> _state { std::make_shared<State>() };
        WhenDestroyed _whenDestroyed { WhenDestroyed::WaitForJob };
        std::jthread _worker {}; //last, so it is stopped before the members it reads are destroyed
    };
}