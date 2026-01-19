#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 

    std::unique_lock<std::mutex> uLock(_mutex);
    _cond.wait(uLock, [this] {
        return !_queue.empty(); 
    });

    // remove last vector element from queue
    T v = std::move(_queue.front());
    _queue.pop_front();

    return v;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.

    // perform vector modification under the lock
    std::lock_guard<std::mutex> uLock(_mutex);

    // add vector to queue
    //std::cout << "    #" << v.getID() << " will be added to the queue" << std::endl;
    _queue.push_back(std::move(msg));
    _cond.notify_one(); // notify client after pushing new Vehicle into vector
}



/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight(){
    _currentPhase = TrafficLightPhase::red;
}

TrafficLight::~TrafficLight() {
    // TODO: Add any necessary cleanup code here
}

void TrafficLight::waitForGreen()
{
    
    while(true) {
        TrafficLightPhase phase = _messageQueue.receive();
        if (phase == TrafficLightPhase::green) {
            return;
        }
    }
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
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

void TrafficLight::nextPhase(){
    if (_currentPhase == TrafficLightPhase::red) {
        _currentPhase = TrafficLightPhase::green;
    }
    else {
        _currentPhase = TrafficLightPhase::red;
    }
}

int getRandomTrafficLightDuration(){
    int MIN = 4000;
    int MAX = 6000;

    std::uniform_int_distribution<> dist(MIN, MAX);

    std::random_device rd;
    std::mt19937 gen(rd());
    
    int random_cycle_duration = dist(gen);

    return random_cycle_duration;
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    int random_cycle_duration = getRandomTrafficLightDuration();
    
    auto lastUpdate = std::chrono::system_clock::now();

    while (true) {

        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        auto now = std::chrono::system_clock::now();
        auto deltaNS = now - lastUpdate;
        auto deltaMS = std::chrono::duration_cast<std::chrono::milliseconds>(deltaNS);

        if (deltaMS.count() >= random_cycle_duration) {
            this->nextPhase();
            _messageQueue.send(std::move(_currentPhase));
            lastUpdate = now;
            random_cycle_duration = getRandomTrafficLightDuration();
        }
    }
}
