"""Type stubs for the C++ core bindings."""

from typing import Any

# ==== memory ====
class MemoryState:
    m: float
    t_last: float
    E_int: float
    e_prev: float
    de_prev: float
    def __init__(self) -> None: ...
    def __repr__(self) -> str: ...

class MemoryParams:
    a: float
    b: float
    c: float
    R_target: float
    eta_plus: float
    eta_minus: float
    m_sat: float
    m_ref: float
    T_0: float
    I_min: float
    I_max: float
    m_max: float
    def __init__(self) -> None: ...

def recall(t: float, a: float, b: float, c: float) -> float: ...
def predict_interval(
    a: float, b: float, c: float,
    R_target: float, I_min: float, I_max: float,
) -> float: ...
def update_strength(
    m: float, score: float, tau_norm: float, params: MemoryParams,
) -> float: ...
def update_state(
    state: MemoryState, score: float, t_response: float,
    answer_length: float, params: MemoryParams,
) -> MemoryState: ...

class _MemoryModule:
    MemoryState: type[MemoryState]
    MemoryParams: type[MemoryParams]
    recall: Any
    predict_interval: Any
    update_strength: Any
    update_state: Any

memory: _MemoryModule

# ==== controller ====
class ControllerState:
    E_int: float
    e_prev: float
    de_prev: float
    def __init__(self) -> None: ...

class ControllerParams:
    Kp: float
    Ki: float
    Kd: float
    gamma: float
    E_max: float
    theta_lapse: float
    R_target: float
    rate_span: float
    rate_floor: float
    m_ref: float
    I_min: float
    I_max: float
    def __init__(self) -> None: ...

class StepResult:
    state: ControllerState
    u: float
    I_new: float
    clamped: bool
    def __init__(self) -> None: ...

class RegulateResult:
    state: ControllerState
    m_new: float
    tau_norm: float
    I_pred: float
    u: float
    I_new: float
    clamped: bool
    def __init__(self) -> None: ...

def apply_rate_limit(
    I_new: float, I_old: float, m: float, params: ControllerParams,
) -> float: ...
def step(
    state: ControllerState, score: float, I_old: float,
    I_pred: float, m: float, params: ControllerParams,
) -> StepResult: ...
def regulate_step(
    ctrl_state: ControllerState,
    m: float,
    score: float,
    t_response: float,
    answer_length: float,
    I_old: float,
    mem_params: MemoryParams,
    ctrl_params: ControllerParams,
) -> RegulateResult: ...

class _ControllerModule:
    ControllerState: type[ControllerState]
    ControllerParams: type[ControllerParams]
    StepResult: type[StepResult]
    RegulateResult: type[RegulateResult]
    apply_rate_limit: Any
    step: Any
    regulate_step: Any

controller: _ControllerModule

# ==== similarity ====
def normalize(s: str) -> str: ...
def utf8_length(s: str) -> int: ...
def levenshtein_distance(a: str, b: str) -> int: ...
def levenshtein_score(a: str, b: str) -> float: ...
def jaro_winkler(a: str, b: str) -> float: ...
def score(a: str, b: str) -> float: ...

class _SimilarityModule:
    normalize: Any
    utf8_length: Any
    levenshtein_distance: Any
    levenshtein_score: Any
    jaro_winkler: Any
    score: Any

similarity: _SimilarityModule