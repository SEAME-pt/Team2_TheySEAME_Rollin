#include <boost/python.hpp>
#include "ActuatorController.hpp"

BOOST_PYTHON_MODULE(actuator) {
	boost::python::class_<ActuatorController>("ActuatorController", boost::python::init<
					RemoteControl*, PurePursuit*>());
}
