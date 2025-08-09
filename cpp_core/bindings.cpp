#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include "folder_watcher.cpp"

namespace py = pybind11;

PYBIND11_MODULE(antnest_core, m)
{
    py::class_<FolderWatcher>(m, "FolderWatcher")
        .def(py::init<const std::wstring &>())
        .def("start", &FolderWatcher::start)
        .def("stop", &FolderWatcher::stop)
        .def("set_callback", &FolderWatcher::setCallback);
}