// The obligatory "Hello World!" example.

#include "caf/actor_ostream.hpp"
#include "caf/actor_system.hpp"
#include "caf/caf_main.hpp"
#include "caf/event_based_actor.hpp"
#include <algorithm>
#include <string>

#include <cctype>    // for std::toupper
using namespace caf;
using namespace std::literals;

using ActorFactory = behavior(*)(event_based_actor*);


behavior Node1(event_based_actor* self) {
    return {
      [self](const std::string& msg) {
           
            std::string ret = msg;
             std::transform(ret.begin(), ret.end(), ret.begin(),
                   [](unsigned char c) { return std::toupper(c); });

           return ret;
      }
    };
}
behavior Node2(event_based_actor* self) {
    return {
      [self](const std::string& msg) {
            
             return std::string{ msg.rbegin(), msg.rend() };
            //return "Worker1 got: " + msg;
          }
    };
}
void Trigger(event_based_actor* self,  std::vector<ActorFactory> factories) {
    
        // Spawn and interact with each actor in order
        for (auto& factory : factories) {
            auto spawned = self->spawn(factory);

            // Send a message to the spawned actor and handle its reply
            self->mail("Trigger")
                .request(spawned, 50s)
                .then([self](const std::string& response) {
                self->println("Response {}", response);
                    });
        }
          //  });
}


void caf_main(actor_system& sys) {
    

    std::vector<ActorFactory> factories = { Node1, Node2 };

    sys.spawn(Trigger,  std::move(factories));
}

// creates a main function for us that calls our caf_main
CAF_MAIN()
