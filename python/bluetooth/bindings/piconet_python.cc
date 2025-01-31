/*
 * Copyright 2022 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

/***********************************************************************************/
/* This file is automatically generated using bindtool and can be manually edited  */
/* The following lines can be configured to regenerate this file during cmake      */
/* If manual edits are made, the following tags should be modified accordingly.    */
/* BINDTOOL_GEN_AUTOMATIC(0)                                                       */
/* BINDTOOL_USE_PYGCCXML(0)                                                        */
/* BINDTOOL_HEADER_FILE(piconet.h)                                        */
/* BINDTOOL_HEADER_FILE_HASH(397481453fe8a6df5928f66604d1150e)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <bluetooth/piconet.h>
// pydoc.h is automatically generated in the build directory
#include <piconet_pydoc.h>

void bind_piconet(py::module& m)
{

    using piconet = ::gr::bluetooth::piconet;
    using basic_rate_piconet = ::gr::bluetooth::basic_rate_piconet;
    using low_energy_piconet = ::gr::bluetooth::low_energy_piconet;


    py::class_<piconet, std::shared_ptr<piconet>>(m, "piconet", D(piconet))

        .def("init_hop_reversal",
             &piconet::init_hop_reversal,
             py::arg("aliased"),
             D(piconet, init_hop_reversal))


        .def("hop", &piconet::hop, py::arg("clock"), D(piconet, hop))


        .def("aliased_channel",
             &piconet::aliased_channel,
             py::arg("channel"),
             D(piconet, aliased_channel))


        .def("reset", &piconet::reset, D(piconet, reset))


        .def("enqueue", &piconet::enqueue, py::arg("pkt"), D(piconet, enqueue))


        .def("dequeue", &piconet::dequeue, D(piconet, dequeue))

        ;


    py::class_<basic_rate_piconet,
               gr::bluetooth::piconet,
               std::shared_ptr<basic_rate_piconet>>(
        m, "basic_rate_piconet", D(basic_rate_piconet))

        .def(py::init(&basic_rate_piconet::make),
             py::arg("LAP"),
             D(basic_rate_piconet, make))


        .def("winnow",
             (int(basic_rate_piconet::*)(int, char)) & basic_rate_piconet::winnow,
             py::arg("offset"),
             py::arg("channel"),
             D(basic_rate_piconet, winnow, 0))


        .def("winnow",
             (int(basic_rate_piconet::*)()) & basic_rate_piconet::winnow,
             D(basic_rate_piconet, winnow, 1))


        .def("get_offset",
             &basic_rate_piconet::get_offset,
             D(basic_rate_piconet, get_offset))


        .def("set_offset",
             &basic_rate_piconet::set_offset,
             py::arg("offset"),
             D(basic_rate_piconet, set_offset))


        .def("get_UAP", &basic_rate_piconet::get_UAP, D(basic_rate_piconet, get_UAP))


        .def("set_UAP",
             &basic_rate_piconet::set_UAP,
             py::arg("uap"),
             D(basic_rate_piconet, set_UAP))


        .def("get_NAP", &basic_rate_piconet::get_NAP, D(basic_rate_piconet, get_NAP))


        .def("set_NAP",
             &basic_rate_piconet::set_NAP,
             py::arg("nap"),
             D(basic_rate_piconet, set_NAP))


        .def("UAP_from_header",
             &basic_rate_piconet::UAP_from_header,
             py::arg("packet"),
             D(basic_rate_piconet, UAP_from_header))


        .def("have_UAP", &basic_rate_piconet::have_UAP, D(basic_rate_piconet, have_UAP))


        .def("have_NAP", &basic_rate_piconet::have_NAP, D(basic_rate_piconet, have_NAP))


        .def(
            "have_clk6", &basic_rate_piconet::have_clk6, D(basic_rate_piconet, have_clk6))


        .def("have_clk27",
             &basic_rate_piconet::have_clk27,
             D(basic_rate_piconet, have_clk27))


        .def("init_hop_reversal",
             &basic_rate_piconet::init_hop_reversal,
             py::arg("aliased"),
             D(basic_rate_piconet, init_hop_reversal))


        .def(
            "hop", &basic_rate_piconet::hop, py::arg("clock"), D(basic_rate_piconet, hop))


        .def("aliased_channel",
             &basic_rate_piconet::aliased_channel,
             py::arg("channel"),
             D(basic_rate_piconet, aliased_channel))


        .def("reset", &basic_rate_piconet::reset, D(basic_rate_piconet, reset))

        ;


    py::class_<low_energy_piconet,
               gr::bluetooth::piconet,
               std::shared_ptr<low_energy_piconet>>(
        m, "low_energy_piconet", D(low_energy_piconet))

        .def(py::init(&low_energy_piconet::make),
             py::arg("aa"),
             D(low_energy_piconet, make))


        .def("init_hop_reversal",
             &low_energy_piconet::init_hop_reversal,
             py::arg("aliased"),
             D(low_energy_piconet, init_hop_reversal))


        .def(
            "hop", &low_energy_piconet::hop, py::arg("clock"), D(low_energy_piconet, hop))


        .def("aliased_channel",
             &low_energy_piconet::aliased_channel,
             py::arg("channel"),
             D(low_energy_piconet, aliased_channel))


        .def("reset", &low_energy_piconet::reset, D(low_energy_piconet, reset))

        ;
}
