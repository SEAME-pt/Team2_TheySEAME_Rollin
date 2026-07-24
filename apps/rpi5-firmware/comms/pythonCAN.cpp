#include <boost/python.hpp>
#include "CAN.hpp"

BOOST_PYTHON_MODULE(comms) {
	boost::python::class_<CAN>("CAN", boost::python::init<
				const std::string&, const unsigned int, unsigned int,
				unsigned int>());
}
