// Unit tests for the viewmodel math (src/core/viewmodel.h). No game needed.
#include <cmath>
#include <cstdio>
#include "core/viewmodel.h"

static int g_pass = 0, g_fail = 0;
#define CHECK(c) do { if (c) ++g_pass; else { ++g_fail; std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #c); } } while (0)

// Viewmodel transform (2026-10-20): seen from the eye, a point keeps its screen direction scaled by k
// (a narrower field of view) and comes s times closer; ScaleAbout keeps its centre fixed.
static void TestViewmodelTransform() {
    using namespace coop::vm;
    float c[3] = {1, 2, 3}, out[3];
    Apply(ScaleAbout(c, 0.3f), c, out);
    CHECK(std::fabs(out[0] - 1) < 1e-5f && std::fabs(out[1] - 2) < 1e-5f && std::fabs(out[2] - 3) < 1e-5f);
    // Camera at (5, 1.6, -2) looking along world -x, up +y, right = fwd x up... any orthonormal frame works.
    float eye[3] = {5, 1.6f, -2}, fwd[3] = {-1, 0, 0}, up[3] = {0, 1, 0}, right[3] = {0, 0, -1};
    float k = FovRatio(100.f, 70.f), s = 0.25f;
    CHECK(k > 1.6f && k < 1.8f);
    Mat34 a = ViewmodelTransform(eye, right, up, fwd, k, s);
    auto cam = [&](const float p[3], float o[3]) {
        float d[3] = {p[0] - eye[0], p[1] - eye[1], p[2] - eye[2]};
        o[0] = d[0] * right[0] + d[1] * right[1] + d[2] * right[2];
        o[1] = d[0] * up[0] + d[1] * up[1] + d[2] * up[2];
        o[2] = d[0] * fwd[0] + d[1] * fwd[1] + d[2] * fwd[2];
    };
    float p[3] = {4.4f, 1.3f, -1.75f}, q[3], pc[3], qc[3];   // a gun 0.6 m ahead, low right
    Apply(a, p, q); cam(p, pc); cam(q, qc);
    CHECK(std::fabs(qc[2] - s * pc[2]) < 1e-4f);                        // s times closer
    CHECK(std::fabs(qc[0] / qc[2] - k * pc[0] / pc[2]) < 1e-4f);       // screen x scaled by k
    CHECK(std::fabs(qc[1] / qc[2] - k * pc[1] / pc[2]) < 1e-4f);       // screen y scaled by k
    Apply(a, eye, out);
    CHECK(std::fabs(out[0] - eye[0]) < 1e-5f && std::fabs(out[1] - eye[1]) < 1e-5f && std::fabs(out[2] - eye[2]) < 1e-5f);
    // Composition: a skinning matrix moved by a, applied to a bind-space vertex, lands at a * (m * v).
    Mat34 m = {{0, -1, 0, 4.5f, 1, 0, 0, 1.2f, 0, 0, 1, -1.9f}};
    float v[3] = {0.1f, 0.05f, 0.2f}, mv[3], amv[3], mv2[3];
    Apply(m, v, mv); Apply(a, mv, amv); Apply(Mul(a, m), v, mv2);
    CHECK(std::fabs(amv[0] - mv2[0]) < 1e-5f && std::fabs(amv[1] - mv2[1]) < 1e-5f && std::fabs(amv[2] - mv2[2]) < 1e-5f);
    // Only the view direction matters: ViewmodelAlong equals ViewmodelTransform.
    Mat34 b = ViewmodelAlong(eye, fwd, k, s);
    for (int i = 0; i < 12; ++i) CHECK(std::fabs(a.m[i] - b.m[i]) < 1e-5f);
    // Object space: a world transform applied to object-space points through the locator.
    float lp[3] = {-67.6f, -6.8f, -205.f}, lq[4] = {0, std::sin(0.4f), 0, std::cos(0.4f)};
    Mat34 l = FromPosQuat(lp, lq), li = InverseRigid(l), back = Mul(li, l);
    for (int i = 0; i < 12; ++i) CHECK(std::fabs(back.m[i] - Identity34().m[i]) < 1e-5f);
    float vo[3] = {0.2f, 1.4f, 0.3f}, w1[3], w2[3], t1[3];
    Apply(InObjectSpace(a, l), vo, t1); Apply(l, t1, w1);   // object-space change, then to world
    Apply(l, vo, t1); Apply(a, t1, w2);                      // to world, then the world change
    CHECK(std::fabs(w1[0] - w2[0]) < 1e-4f && std::fabs(w1[1] - w2[1]) < 1e-4f && std::fabs(w1[2] - w2[2]) < 1e-4f);
    // The engine's row-vector 4x4 with scale: same point either way, and a true inverse.
    float r4[16] = {0.f, 0, -1.2f, 0,   0, 1.2f, 0, 0,   1.2f, 0, 0, 0,   -67.6f, -6.8f, -205.f, 1};
    Mat34 ls = FromRowVector4x4(r4), lsi = InverseAffine(ls), b2 = Mul(lsi, ls);
    float pr[3] = {0.3f, 1.1f, -0.2f}, rv[3], cv[3];
    for (int i = 0; i < 3; ++i) rv[i] = pr[0] * r4[0 + i] + pr[1] * r4[4 + i] + pr[2] * r4[8 + i] + r4[12 + i];   // row vector * 4x4
    Apply(ls, pr, cv);
    CHECK(std::fabs(rv[0] - cv[0]) < 1e-4f && std::fabs(rv[1] - cv[1]) < 1e-4f && std::fabs(rv[2] - cv[2]) < 1e-4f);
    for (int i = 0; i < 12; ++i) CHECK(std::fabs(b2.m[i] - Identity34().m[i]) < 1e-4f);
    Apply(InObjectSpaceAffine(a, ls), pr, t1); Apply(ls, t1, w1);
    Apply(ls, pr, t1); Apply(a, t1, w2);
    CHECK(std::fabs(w1[0] - w2[0]) < 1e-3f && std::fabs(w1[1] - w2[1]) < 1e-3f && std::fabs(w1[2] - w2[2]) < 1e-3f);
}

// Head-turn limit: the camera's yaw is held within the limit of the body's facing; pitch is kept.
static void TestClampYaw() {
    using namespace coop::vm;
    const float deg = 3.14159265f / 180.f;
    auto camQuat = [](float yaw, float pitch, float q[4]) {   // forward (cos p sin y, sin p, cos p cos y)
        float a = -pitch;   // about x: +z goes to (0, -sin a, cos a)
        float qy[4] = {0, std::sin(yaw / 2), 0, std::cos(yaw / 2)}, qx[4] = {std::sin(a / 2), 0, 0, std::cos(a / 2)};
        q[0] = qy[3] * qx[0] + qy[0] * qx[3] + qy[1] * qx[2] - qy[2] * qx[1];
        q[1] = qy[3] * qx[1] - qy[0] * qx[2] + qy[1] * qx[3] + qy[2] * qx[0];
        q[2] = qy[3] * qx[2] + qy[0] * qx[1] - qy[1] * qx[0] + qy[2] * qx[3];
        q[3] = qy[3] * qx[3] - qy[0] * qx[0] - qy[1] * qx[1] - qy[2] * qx[2];
    };
    auto pitchOf = [](const float q[4]) { return std::asin(2 * (q[1] * q[2] - q[3] * q[0])); };   // forward y
    float q[4];
    camQuat(10 * deg, 20 * deg, q);
    CHECK(std::fabs(YawOfQuat(q) - 10 * deg) < 1e-4f);
    CHECK(std::fabs(pitchOf(q) - 20 * deg) < 1e-4f);
    CHECK(ClampYaw(q, 0, 65 * deg) == 0);                                  // inside: untouched
    camQuat(150 * deg, -30 * deg, q);                                       // looking back over the right shoulder
    float t = ClampYaw(q, 40 * deg, 65 * deg);
    CHECK(std::fabs(t + 45 * deg) < 1e-3f);
    CHECK(std::fabs(YawOfQuat(q) - 105 * deg) < 1e-3f);                     // body 40 + 65
    CHECK(std::fabs(pitchOf(q) + 30 * deg) < 1e-3f);                        // pitch kept
    camQuat(-170 * deg, 0, q);                                              // across the +-180 seam
    ClampYaw(q, 170 * deg, 65 * deg);                                       // -170 is 20 deg from 170: inside
    CHECK(std::fabs(WrapPi(YawOfQuat(q) + 170 * deg)) < 1e-3f);
    camQuat(60 * deg, 0, q);
    ClampYaw(q, 170 * deg, 65 * deg);                                       // 110 deg off: to 170 - 65
    CHECK(std::fabs(YawOfQuat(q) - 105 * deg) < 1e-3f);
    float n = q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3];
    CHECK(std::fabs(n - 1) < 1e-4f);
}

// Smooth head lock: soft edge, no snap when it starts past the limit, eased release.
static void TestYawLimiter() {
    using namespace coop::vm;
    const float deg = 3.14159265f / 180.f, L = 65 * deg, m = 12 * deg;
    CHECK(SoftLimit(30 * deg, L, m) == 30 * deg);                                  // inside the knee: untouched
    float prev = 0;
    for (float d = 0; d < 180 * deg; d += 1 * deg) {                               // monotonic, below the limit
        float o = SoftLimit(d, L, m);
        CHECK(o >= prev - 1e-6f && o < L);
        prev = o;
    }
    CHECK(std::fabs(SoftLimit(-100 * deg, L, m) + SoftLimit(100 * deg, L, m)) < 1e-6f);
    YawLimiter y;
    CHECK(y.Step(100 * deg, false, L, 0.016f, 180 * deg, 300 * deg, m) == 100 * deg);   // unlocked: free
    float first = y.Step(100 * deg, true, L, 0.016f, 180 * deg, 300 * deg, m);          // lock starts 100 deg off
    CHECK(first > 90 * deg);                                                             // no snap to 65
    float o = first;
    for (int i = 0; i < 60; ++i) o = y.Step(100 * deg, true, L, 0.016f, 180 * deg, 300 * deg, m);
    CHECK(o < L && o > 55 * deg);                                                        // settled at the soft limit
    float r1 = y.Step(100 * deg, false, L, 0.016f, 180 * deg, 300 * deg, m);            // released
    CHECK(r1 < 75 * deg);                                                                // eases out, no jump to 100
    for (int i = 0; i < 60; ++i) r1 = y.Step(100 * deg, false, L, 0.016f, 180 * deg, 300 * deg, m);
    CHECK(std::fabs(r1 - 100 * deg) < 1e-5f);                                            // back to free
}

int main() {
    TestViewmodelTransform();
    TestClampYaw();
    TestYawLimiter();
    std::printf("%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
