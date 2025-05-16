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
             self->quit();
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

behavior Trigger(event_based_actor* self,  std::vector<ActorFactory> factories) {
    
      
    self->mail("Trigger",0).send(self);
	return 
    {
		[self,factories](const std::string& msg, int index) {
			self->println("got: {} {}", msg, index);
            if(index >=2)
				return;
			
           // self->mail(msg, index+1).send(self);
            auto spawned = self->spawn(factories[index]);
            self->mail(msg).request(spawned, 50s)
                .then([self,index](const std::string& response) {
                self->println("Response {}", response);
                self->mail(response, index+1).send(self);
                //anon_send_exit(spawned, exit_reason::normal);
                    });
           
		}
	};
}


void caf_main(actor_system& sys) {
    

    std::vector<ActorFactory> factories = { Node1, Node2 };

    sys.spawn(Trigger,  std::move(factories));
}

// creates a main function for us that calls our caf_main
CAF_MAIN()
