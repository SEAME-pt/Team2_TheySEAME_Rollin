#include <boost/python.hpp>
#include "Bridge.hpp"

BOOST_PYTHON_MODULE(actuator) {
	boost::python::class_<Bridge>("Bridge", boost::python::init<LkaControl*>());
}
