"""Smoke test for the C++ core bindings."""
from backend.shared.native import llp_core


def main():
    # --- memory ---
    I = llp_core.memory.predict_interval(0.0, 1.0, 0.5, 0.9, 1.0, 365.0)
    print(f"predict_interval = {I:.4f}  (expected ~1.2346)")

    r = llp_core.memory.recall(4.0, 0.0, 1.0, 0.5)
    print(f"recall(4) = {r:.4f}  (expected 0.5)")

    # --- controller ---
    cparams = llp_core.controller.ControllerParams()
    cstate = llp_core.controller.ControllerState()

    result = llp_core.controller.step(
        cstate, score=1.0, I_old=10.0, I_pred=10.0, m=5.0, params=cparams
    )
    print(f"PID success: I_new = {result.I_new:.4f}, u = {result.u:.4f}")

    result2 = llp_core.controller.step(
        cstate, score=0.0, I_old=10.0, I_pred=10.0, m=5.0, params=cparams
    )
    print(f"PID fail:    I_new = {result2.I_new:.4f}, u = {result2.u:.4f}")

    assert 1.2 < I < 1.3
    assert abs(r - 0.5) < 1e-9
    assert result.I_new > 10.0
    assert result2.I_new < 10.0
    print("\nAll native bindings OK.")


if __name__ == "__main__":
    main()