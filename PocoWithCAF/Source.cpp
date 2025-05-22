#include "PocoOpenAiClient.h"
#include "caf/actor_system.hpp"
#include "caf/event_based_actor.hpp"
#include "caf/caf_main.hpp"
#include <iostream>
#include <string>  
using namespace caf;
using namespace std::literals;
 //OpenAI Actor: handles one request, replies, then quits
behavior OpenAINode(event_based_actor* self) {
    
    return {
        
        [self](const std::string& prompt) mutable {
            OpenAIClient client;
            std::string response = client.generateCompletion(prompt);
            self->quit();
            return response;
        }
    };
}

//// Trigger Actor: sends prompt to OpenAINode and prints response
behavior Trigger(event_based_actor* self) {
    self->mail("What is C++").send(self);
    return {
        [self](const std::string& prompt) {
            auto openaiHandle = self->spawn(OpenAINode);
            self->mail(prompt).request(openaiHandle, 50s)
                .then([self](const std::string& response) {
                self->println("Response {}", response);
                    self->quit();
                });
        }
    };
}

behavior mirror(event_based_actor* self) {
    // return the (initial) actor behavior
    return {
        // a handler for messages containing a single string
        // that replies with a string
        [self](const std::string& what) -> std::string {
            // prints "Hello World!" (thread-safe)
            self->println("{}", what);
            // reply "!dlroW olleH"
            return std::string{what.rbegin(), what.rend()};
          },
    };
}


void hello_world(event_based_actor* self, const actor& buddy) {
    // send "Hello World!" to our buddy ...
    self->mail("Hello World!")
        .request(buddy, 10s)
        .then(
            // ... wait up to 10s for a response ...
            [self](const std::string& what) {
                // ... and print it
                self->println("{}", what);
            });
}


void caf_main(actor_system& sys) 

{
    auto mirror_actor = sys.spawn(Trigger);
 
}
CAF_MAIN()
