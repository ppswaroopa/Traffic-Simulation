#include <iostream>
#include <random>
#include <thread>
#include <chrono>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> lock(_mutex);
    _condition.wait(lock, [this] { return !_queue.empty(); });
    
    T msg = std::move(_queue.front());
    _queue.pop_front();

    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lock(_mutex);
    _queue.emplace_back(std::move(msg));
    _condition.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (_messageQueue.receive() == TrafficLightPhase::green)
        {
            return;
        }
    }
    return;
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    std::random_device rdCycleDur;
    std::mt19937 gen(rdCycleDur());
    std::uniform_int_distribution<> dist(4,6);
    auto cycle_duration = std::chrono::seconds(dist(gen));

    auto tik = std::chrono::system_clock::now();

    while (true)
    {
        // Wait for 1ms
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        auto tok = std::chrono::system_clock::now() - tik;
        
        if (tok >= cycle_duration)
        {
            // Toggle current phase
            switch (_currentPhase)
            {
            case red:
                _currentPhase = green;
                break;
            case green:
                _currentPhase = red;
                // std::cout<<"Turned RED\n";
                break;
            default:
                _currentPhase = red;
                break;
            }
            // _currentPhase = (_currentPhase == TrafficLightPhase::red) ? TrafficLightPhase::green : TrafficLightPhase::red;
            
            _messageQueue.send(std::move(_currentPhase));
            
            //update cycle end time
            tik = std::chrono::system_clock::now();
        }
        
    }
    
}

