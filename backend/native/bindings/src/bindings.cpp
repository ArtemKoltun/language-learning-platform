#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "llp/memory/memory.hpp"
#include "llp/controller/controller.hpp"
#include "llp/similarity/similarity.hpp"

namespace py = pybind11;
using namespace llp;

PYBIND11_MODULE(llp_core, m) {
    m.doc() = "Language Learning Platform — C++ core";

    // ==================== memory ====================
    py::module_ mem = m.def_submodule("memory", "Memory model");

    py::class_<memory::MemoryState>(mem, "MemoryState")
        .def(py::init<>())
        .def_readwrite("m", &memory::MemoryState::m)
        .def_readwrite("t_last", &memory::MemoryState::t_last)
        .def_readwrite("E_int", &memory::MemoryState::E_int)
        .def_readwrite("e_prev", &memory::MemoryState::e_prev)
        .def_readwrite("de_prev", &memory::MemoryState::de_prev)
        .def("__repr__", [](const memory::MemoryState& s) {
            return "<MemoryState m=" + std::to_string(s.m) + ">";
        });

    py::class_<memory::MemoryParams>(mem, "MemoryParams")
        .def(py::init<>())
        .def_readwrite("a", &memory::MemoryParams::a)
        .def_readwrite("b", &memory::MemoryParams::b)
        .def_readwrite("c", &memory::MemoryParams::c)
        .def_readwrite("R_target", &memory::MemoryParams::R_target)
        .def_readwrite("eta_plus", &memory::MemoryParams::eta_plus)
        .def_readwrite("eta_minus", &memory::MemoryParams::eta_minus)
        .def_readwrite("m_sat", &memory::MemoryParams::m_sat)
        .def_readwrite("m_ref", &memory::MemoryParams::m_ref)
        .def_readwrite("T_0", &memory::MemoryParams::T_0)
        .def_readwrite("I_min", &memory::MemoryParams::I_min)
        .def_readwrite("I_max", &memory::MemoryParams::I_max)
        .def_readwrite("m_max", &memory::MemoryParams::m_max);

    mem.def("recall", &memory::recall,
            py::arg("t"), py::arg("a"), py::arg("b"), py::arg("c"),
            "Probability of recall at time t");

    mem.def("predict_interval", &memory::predict_interval,
            py::arg("a"), py::arg("b"), py::arg("c"),
            py::arg("R_target"), py::arg("I_min"), py::arg("I_max"),
            "Predicted interval for target retention");

    mem.def("update_strength", &memory::update_strength,
            py::arg("m"), py::arg("score"), py::arg("tau_norm"),
            py::arg("params"),
            "Update memory strength after a review");

    mem.def("update_state", &memory::update_state,
            py::arg("state"), py::arg("score"),
            py::arg("t_response"), py::arg("answer_length"),
            py::arg("params"),
            "Update full memory state after a review");

    // ==================== controller ====================
    py::module_ ctrl = m.def_submodule("controller", "PID controller");

    py::class_<controller::ControllerState>(ctrl, "ControllerState")
        .def(py::init<>())
        .def_readwrite("E_int", &controller::ControllerState::E_int)
        .def_readwrite("e_prev", &controller::ControllerState::e_prev)
        .def_readwrite("de_prev", &controller::ControllerState::de_prev);

    py::class_<controller::ControllerParams>(ctrl, "ControllerParams")
        .def(py::init<>())
        .def_readwrite("Kp", &controller::ControllerParams::Kp)
        .def_readwrite("Ki", &controller::ControllerParams::Ki)
        .def_readwrite("Kd", &controller::ControllerParams::Kd)
        .def_readwrite("gamma", &controller::ControllerParams::gamma)
        .def_readwrite("E_max", &controller::ControllerParams::E_max)
        .def_readwrite("theta_lapse", &controller::ControllerParams::theta_lapse)
        .def_readwrite("R_target", &controller::ControllerParams::R_target)
        .def_readwrite("rate_span", &controller::ControllerParams::rate_span)
        .def_readwrite("rate_floor", &controller::ControllerParams::rate_floor)
        .def_readwrite("m_ref", &controller::ControllerParams::m_ref)
        .def_readwrite("I_min", &controller::ControllerParams::I_min)
        .def_readwrite("I_max", &controller::ControllerParams::I_max);

    py::class_<controller::StepResult>(ctrl, "StepResult")
        .def(py::init<>())
        .def_readwrite("state", &controller::StepResult::state)
        .def_readwrite("u", &controller::StepResult::u)
        .def_readwrite("I_new", &controller::StepResult::I_new)
        .def_readwrite("clamped", &controller::StepResult::clamped);

    py::class_<controller::RegulateResult>(ctrl, "RegulateResult")
        .def(py::init<>())
        .def_readwrite("state", &controller::RegulateResult::state)
        .def_readwrite("m_new", &controller::RegulateResult::m_new)
        .def_readwrite("tau_norm", &controller::RegulateResult::tau_norm)
        .def_readwrite("I_pred", &controller::RegulateResult::I_pred)
        .def_readwrite("u", &controller::RegulateResult::u)
        .def_readwrite("I_new", &controller::RegulateResult::I_new)
        .def_readwrite("clamped", &controller::RegulateResult::clamped);

    ctrl.def("apply_rate_limit", &controller::apply_rate_limit,
             py::arg("I_new"), py::arg("I_old"), py::arg("m"),
             py::arg("params"),
             "Apply rate limiting with gain scheduling");

    ctrl.def("step", &controller::step,
             py::arg("state"), py::arg("score"),
             py::arg("I_old"), py::arg("I_pred"), py::arg("m"),
             py::arg("params"),
             "One PID step");

    ctrl.def("regulate_step", &controller::regulate_step,
             py::arg("ctrl_state"), py::arg("m"), py::arg("score"),
             py::arg("t_response"), py::arg("answer_length"),
             py::arg("I_old"),
             py::arg("mem_params"), py::arg("ctrl_params"),
             "Full 2DOF step: feedforward (memory) + feedback (PID)");

    // ==================== similarity ====================
    py::module_ sim = m.def_submodule("similarity", "String similarity");

    sim.def("normalize", &similarity::normalize,
            py::arg("s"),
            "Normalize string: lowercase, trim, strip ASCII punctuation");

    sim.def("utf8_length", &similarity::utf8_length,
            py::arg("s"),
            "Length in UTF-8 codepoints");

    sim.def("levenshtein_distance", &similarity::levenshtein_distance,
            py::arg("a"), py::arg("b"),
            "Levenshtein distance in codepoints");

    sim.def("levenshtein_score", &similarity::levenshtein_score,
            py::arg("a"), py::arg("b"),
            "Normalized Levenshtein score 0..1");

    sim.def("jaro_winkler", &similarity::jaro_winkler,
            py::arg("a"), py::arg("b"),
            "Jaro-Winkler similarity 0..1");

    sim.def("score", &similarity::score,
            py::arg("a"), py::arg("b"),
            "Combined similarity score 0..1");
}