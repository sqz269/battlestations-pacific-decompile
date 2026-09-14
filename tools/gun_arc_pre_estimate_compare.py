"""Compare 006DF520 step 6's elevation pre-estimate with the recovered arc 00955630.

Evidence for the correction appended to docs/GAME_EXECUTABLE.md and for
docs/GUN_GRAVITY_ARC.md. Run it with no arguments:

    python tools/gun_arc_pre_estimate_compare.py

Part 1 shows the two agree to machine precision for a target at the muzzle's own
height, which is an identity rather than a numerical accident: with s = g*R/v^2
and h = 0, (R - sqrt(D))/(2k) = (1 - sqrt(1 - s^2))/s = tan(asin(s)/2).
Part 2 shows they diverge only through the height difference h, and that the
vertical error at the target is about h itself, nearly independently of range.
"""

import math

# the double at 00CF9058, 9.81f widened
g = 9.8100004196166992

print('--- part 1: h = 0, the two are the same function ---')
def pre(R, v):            # min(asin(min(R*g/v^2,1))/2, pi/4)
    s = R*g/(v*v)
    return min(math.asin(min(s,1.0))*0.5, math.pi/4)
def exact(R, v, h=0.0):   # tan(t) = (R - sqrt(D))/(2k), k = g*R^2/(2v^2)
    k = (R*R*g)/(v*(v+v))
    D = R*R - 4.0*k*(k+h)
    if D < 0: return None
    return math.atan((R - math.sqrt(D))/(2.0*k))
print(f"{'v':>5} {'R':>6} {'Rmax':>7} {'pre(mrad)':>10} {'exact':>9} {'diff mrad':>10} {'miss m':>8}")
for v in (250.0, 400.0, 800.0):
    Rmax = v*v/g
    for frac in (0.1, 0.25, 0.5, 0.75, 0.9, 0.99):
        R = Rmax*frac
        a, b = pre(R, v), exact(R, v)
        if b is None: continue
        d = b-a
        print(f"{v:5.0f} {R:6.0f} {Rmax:7.0f} {a*1000:10.3f} {b*1000:9.3f} {d*1000:10.4f} {abs(d)*R:8.2f}")

print()
print('--- part 2: the divergence is the height difference ---')
def _pre_unused(R, v):
    s = R*g/(v*v)
    return min(math.asin(min(s,1.0))*0.5, math.pi/4)
def exact_h(R, v, h):
    k = (R*R*g)/(v*(v+v))
    D = R*R - 4.0*k*(k+h)
    if D < 0: return None
    return math.atan((R - math.sqrt(D))/(2.0*k))
v = 400.0
print(f"{'h (m)':>8} {'R (m)':>7} {'pre mrad':>9} {'exact':>9} {'diff mrad':>10} {'vert miss m':>12}")
for h in (0.0, 5.0, 20.0, 100.0, 1000.0, -1000.0):
    for R in (2000.0, 5000.0, 10000.0):
        a, b = pre(R, v), exact_h(R, v, h)
        if b is None:
            print(f"{h:8.0f} {R:7.0f} {a*1000:9.3f} {'no sol':>9}")
            continue
        d = b - a
        print(f"{h:8.0f} {R:7.0f} {a*1000:9.3f} {b*1000:9.3f} {d*1000:10.4f} {abs(d)*R:12.2f}")
