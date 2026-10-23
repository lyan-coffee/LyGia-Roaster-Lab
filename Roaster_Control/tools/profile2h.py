#!/usr/bin/env python3
# Doi cac file profile JSON (dinh dang lygia-profile) thanh lib/RoasterProfileData.h
# Dung: python tools/profile2h.py out.h profile/a.profile.json [b.profile.json ...]   (khong dau, de dung cho Arduino)
# JSON doc bang UTF-8. Profile khong co "ref" nhung du moc TP / DE / FC1s / D (bt > 0) thi tu sinh duong mau BT / RoR (ham synth).
import json, sys, re

# Sinh duong mau BT/RoR tu cac moc (Charge, TP, DE, FC, Drop) - thuan Python, khong can numpy.
# RoR sau TP = ramp(x) * (a + b*x + c*x^2), x = giay ke tu TP, ramp tang 0 -> 1 trong RISE_S giay.
# a, b, c giai sao cho tich phan RoR moi doan TP-DE, DE-FC, FC-Drop dung bang do tang BT -> duong BT di qua dung cac moc.
# Truoc TP: BT = TP + (Charge - TP) * (1 - t/tTP)^3 (do doc xuong, cham day tai TP).
RISE_S = 45.0

def _solve3(M, y):
    A = [M[i][:] + [y[i]] for i in range(3)]
    for c in range(3):
        p = max(range(c, 3), key=lambda r: abs(A[r][c])); A[c], A[p] = A[p], A[c]
        for r in range(3):
            if r != c:
                f = A[r][c] / A[c][c]
                for k in range(c, 4): A[r][k] -= f * A[c][k]
    return [A[i][3] / A[i][i] for i in range(3)]

def _ramp(x):
    if x <= 0: return 0.0
    if x >= RISE_S: return 1.0
    u = x / RISE_S; return u * u * (3 - 2 * u)          # smoothstep

def _integ(f, a, b, n=400):
    h = (b - a) / n; return sum(f(a + (i + 0.5) * h) for i in range(n)) * h

def synth(charge, tp, de, fc, drop, step=5):
    """tp/de/fc/drop = (giay, BT). Tra ve (bt[], ror[]) moi `step` giay; ror = C/phut, None truoc TP."""
    t0 = tp[0]
    basis = [lambda x: _ramp(x), lambda x: _ramp(x) * x, lambda x: _ramp(x) * x * x]
    M = []; y = []
    for a, b in ((tp, de), (de, fc), (fc, drop)):
        M.append([_integ(f, a[0] - t0, b[0] - t0) / 60.0 for f in basis]); y.append(b[1] - a[1])
    c = _solve3(M, y)
    ror_x = lambda x: sum(ci * f(x) for ci, f in zip(c, basis))
    ts = list(range(0, int(drop[0]) + 1, step))
    if ts[-1] < drop[0]: ts.append(ts[-1] + step)
    bt = []; ror = []
    for t in ts:
        if t <= t0:
            bt.append(round(tp[1] + (charge - tp[1]) * (1 - t / t0) ** 3, 1)); ror.append(None)
        else:
            bt.append(round(tp[1] + _integ(ror_x, 0, t - t0, 200) / 60.0, 1)); ror.append(round(ror_x(t - t0), 1))
    return bt, ror, ror_x

TRIG = {"PREHEAT":"PT_PREHEAT","CHARGE":"PT_CHARGE","TP":"PT_TP","DE":"PT_DE","FC1S":"PT_FC1S","DROP":"PT_DROP"}
def cond(rule):
    a = rule["when"]["any"][0]
    if "ev" in a: return TRIG[a["ev"]], 0
    return "PT_BT_GT", int(round(a["bt_gt"]*10))
def acts(rule):
    out = []
    for d in rule["do"]:
        if "pid" in d:
            out.append(("PA_PID_ON", int(round(d["set_c"]*10))) if d["pid"] else ("PA_PID_OFF", 0))
        elif "heater" in d: out.append(("PA_HEATER", d["heater"]))
        elif "fan" in d: out.append(("PA_FAN", d["fan"]))
    return out
def cstr(s): return '"' + s.replace('\\','\\\\').replace('"','\\"') + '"'
def main(out, files):
    L = ['/* Tu dong sinh boi tools/profile2h.py - KHONG sua tay. Du lieu Profile rang mau (ban goc, nam trong flash). */',
         '#pragma once', '#if ENABLE_PROFILE', '']
    names = []
    for n, f in enumerate(files):
        p = json.load(open(f, encoding="utf-8")); k = "P%d" % n
        ref = p.get("ref")
        mk = p.get("markers") or {}
        if not ref and all(mk.get(x, {}).get("bt", 0) > 0 for x in ("TP", "DE", "FC1s", "D")):   # khong co duong mau: sinh tu cac moc
            g = lambda x: (mk[x]["t"], mk[x]["bt"])
            sbt, sror, _ = synth(p["charge_temp_c"], g("TP"), g("DE"), g("FC1s"), g("D"))
            ref = {"step_s": 5, "bt": sbt, "ror": sror}
        if len(p["name"]) > 20: print("CANH BAO: ten '%s' dai %d ky tu (toi da 20)" % (p["name"], len(p["name"])))
        bt = [int(round(v*10)) for v in ref["bt"]] if ref else [0]
        ror = [(-32768 if v is None else int(round(v*10))) for v in ref["ror"]] if ref else [-32768]
        L.append("static const int16_t %s_BT[] = {%s};" % (k, ",".join(map(str, bt))))
        L.append("static const int16_t %s_ROR[] = {%s};" % (k, ",".join(map(str, ror))))
        rs = []
        for r in p["rules"]:
            t, v = cond(r); a = acts(r)
            after = 1 if any("after" in c for c in r["when"]["all"]) else 0
            a += [("PA_NONE", 0)] * (3 - len(a))
            rs.append("{%s,%d,%d,%d,{%s}}" % (t, after, v, sum(1 for x in a if x[0] != "PA_NONE"),
                      ",".join("{%s,%d}" % x for x in a)))
        L.append("static const ProfRule %s_RULES[] = {%s};" % (k, ",\n  ".join(rs)))
        m = p.get("markers") or {"TP": {"t": 0, "bt": 0}, "DE": {"t": 0, "bt": 0}, "FC1s": {"t": 0, "bt": 0}}; am = p.get("auto_mark", {})
        names.append("""  { %s, %s, %s, %s, %d, %d, %d, %d, %d, {%d,%d}, {%d,%d}, {%d,%d}, %d, %d, %s_BT, %s_ROR, %d, %s_RULES }""" % (
            cstr(p["name"]), cstr(p.get("weight_note","")), cstr(p.get("process","")), cstr(p.get("level","")), int(round(p["charge_temp_c"]*10)),
            int(round(p["preheat"]["pid_c"]*10)), p["preheat"]["fan"],
            1 if am.get("DE", True) else 0, 1 if am.get("FC1s", False) else 0,
            m["TP"]["t"], int(round(m["TP"]["bt"]*10)), m["DE"]["t"], int(round(m["DE"]["bt"]*10)),
            m["FC1s"]["t"], int(round(m["FC1s"]["bt"]*10)),
            ref["step_s"] if ref else 5, len(bt) if ref else 0, k, k, len(p["rules"]), k))
        L.append("")
    L.append("static const ProfileDef PROFILE_FACTORY[] = {\n" + ",\n".join(names) + "\n};")
    L.append("#define PROFILE_COUNT ((int)(sizeof(PROFILE_FACTORY) / sizeof(PROFILE_FACTORY[0])))")
    L.append("#endif")
    open(out, "w", newline="\n").write("\n".join(L) + "\n")
if __name__ == "__main__": main(sys.argv[1], sys.argv[2:])
