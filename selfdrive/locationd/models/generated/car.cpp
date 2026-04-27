#include "car.h"

namespace {
#define DIM 9
#define EDIM 9
#define MEDIM 9
typedef void (*Hfun)(double *, double *, double *);

double mass;

void set_mass(double x){ mass = x;}

double rotational_inertia;

void set_rotational_inertia(double x){ rotational_inertia = x;}

double center_to_front;

void set_center_to_front(double x){ center_to_front = x;}

double center_to_rear;

void set_center_to_rear(double x){ center_to_rear = x;}

double stiffness_front;

void set_stiffness_front(double x){ stiffness_front = x;}

double stiffness_rear;

void set_stiffness_rear(double x){ stiffness_rear = x;}
const static double MAHA_THRESH_25 = 3.8414588206941227;
const static double MAHA_THRESH_24 = 5.991464547107981;
const static double MAHA_THRESH_30 = 3.8414588206941227;
const static double MAHA_THRESH_26 = 3.8414588206941227;
const static double MAHA_THRESH_27 = 3.8414588206941227;
const static double MAHA_THRESH_29 = 3.8414588206941227;
const static double MAHA_THRESH_28 = 3.8414588206941227;
const static double MAHA_THRESH_31 = 3.8414588206941227;

/******************************************************************************
 *                      Code generated with SymPy 1.14.0                      *
 *                                                                            *
 *              See http://www.sympy.org/ for more information.               *
 *                                                                            *
 *                         This file is part of 'ekf'                         *
 ******************************************************************************/
void err_fun(double *nom_x, double *delta_x, double *out_4898271858899960872) {
   out_4898271858899960872[0] = delta_x[0] + nom_x[0];
   out_4898271858899960872[1] = delta_x[1] + nom_x[1];
   out_4898271858899960872[2] = delta_x[2] + nom_x[2];
   out_4898271858899960872[3] = delta_x[3] + nom_x[3];
   out_4898271858899960872[4] = delta_x[4] + nom_x[4];
   out_4898271858899960872[5] = delta_x[5] + nom_x[5];
   out_4898271858899960872[6] = delta_x[6] + nom_x[6];
   out_4898271858899960872[7] = delta_x[7] + nom_x[7];
   out_4898271858899960872[8] = delta_x[8] + nom_x[8];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_6923660614592572242) {
   out_6923660614592572242[0] = -nom_x[0] + true_x[0];
   out_6923660614592572242[1] = -nom_x[1] + true_x[1];
   out_6923660614592572242[2] = -nom_x[2] + true_x[2];
   out_6923660614592572242[3] = -nom_x[3] + true_x[3];
   out_6923660614592572242[4] = -nom_x[4] + true_x[4];
   out_6923660614592572242[5] = -nom_x[5] + true_x[5];
   out_6923660614592572242[6] = -nom_x[6] + true_x[6];
   out_6923660614592572242[7] = -nom_x[7] + true_x[7];
   out_6923660614592572242[8] = -nom_x[8] + true_x[8];
}
void H_mod_fun(double *state, double *out_2317713943645087999) {
   out_2317713943645087999[0] = 1.0;
   out_2317713943645087999[1] = 0.0;
   out_2317713943645087999[2] = 0.0;
   out_2317713943645087999[3] = 0.0;
   out_2317713943645087999[4] = 0.0;
   out_2317713943645087999[5] = 0.0;
   out_2317713943645087999[6] = 0.0;
   out_2317713943645087999[7] = 0.0;
   out_2317713943645087999[8] = 0.0;
   out_2317713943645087999[9] = 0.0;
   out_2317713943645087999[10] = 1.0;
   out_2317713943645087999[11] = 0.0;
   out_2317713943645087999[12] = 0.0;
   out_2317713943645087999[13] = 0.0;
   out_2317713943645087999[14] = 0.0;
   out_2317713943645087999[15] = 0.0;
   out_2317713943645087999[16] = 0.0;
   out_2317713943645087999[17] = 0.0;
   out_2317713943645087999[18] = 0.0;
   out_2317713943645087999[19] = 0.0;
   out_2317713943645087999[20] = 1.0;
   out_2317713943645087999[21] = 0.0;
   out_2317713943645087999[22] = 0.0;
   out_2317713943645087999[23] = 0.0;
   out_2317713943645087999[24] = 0.0;
   out_2317713943645087999[25] = 0.0;
   out_2317713943645087999[26] = 0.0;
   out_2317713943645087999[27] = 0.0;
   out_2317713943645087999[28] = 0.0;
   out_2317713943645087999[29] = 0.0;
   out_2317713943645087999[30] = 1.0;
   out_2317713943645087999[31] = 0.0;
   out_2317713943645087999[32] = 0.0;
   out_2317713943645087999[33] = 0.0;
   out_2317713943645087999[34] = 0.0;
   out_2317713943645087999[35] = 0.0;
   out_2317713943645087999[36] = 0.0;
   out_2317713943645087999[37] = 0.0;
   out_2317713943645087999[38] = 0.0;
   out_2317713943645087999[39] = 0.0;
   out_2317713943645087999[40] = 1.0;
   out_2317713943645087999[41] = 0.0;
   out_2317713943645087999[42] = 0.0;
   out_2317713943645087999[43] = 0.0;
   out_2317713943645087999[44] = 0.0;
   out_2317713943645087999[45] = 0.0;
   out_2317713943645087999[46] = 0.0;
   out_2317713943645087999[47] = 0.0;
   out_2317713943645087999[48] = 0.0;
   out_2317713943645087999[49] = 0.0;
   out_2317713943645087999[50] = 1.0;
   out_2317713943645087999[51] = 0.0;
   out_2317713943645087999[52] = 0.0;
   out_2317713943645087999[53] = 0.0;
   out_2317713943645087999[54] = 0.0;
   out_2317713943645087999[55] = 0.0;
   out_2317713943645087999[56] = 0.0;
   out_2317713943645087999[57] = 0.0;
   out_2317713943645087999[58] = 0.0;
   out_2317713943645087999[59] = 0.0;
   out_2317713943645087999[60] = 1.0;
   out_2317713943645087999[61] = 0.0;
   out_2317713943645087999[62] = 0.0;
   out_2317713943645087999[63] = 0.0;
   out_2317713943645087999[64] = 0.0;
   out_2317713943645087999[65] = 0.0;
   out_2317713943645087999[66] = 0.0;
   out_2317713943645087999[67] = 0.0;
   out_2317713943645087999[68] = 0.0;
   out_2317713943645087999[69] = 0.0;
   out_2317713943645087999[70] = 1.0;
   out_2317713943645087999[71] = 0.0;
   out_2317713943645087999[72] = 0.0;
   out_2317713943645087999[73] = 0.0;
   out_2317713943645087999[74] = 0.0;
   out_2317713943645087999[75] = 0.0;
   out_2317713943645087999[76] = 0.0;
   out_2317713943645087999[77] = 0.0;
   out_2317713943645087999[78] = 0.0;
   out_2317713943645087999[79] = 0.0;
   out_2317713943645087999[80] = 1.0;
}
void f_fun(double *state, double dt, double *out_7025688067405742044) {
   out_7025688067405742044[0] = state[0];
   out_7025688067405742044[1] = state[1];
   out_7025688067405742044[2] = state[2];
   out_7025688067405742044[3] = state[3];
   out_7025688067405742044[4] = state[4];
   out_7025688067405742044[5] = dt*((-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]))*state[6] - 9.8100000000000005*state[8] + stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*state[1]) + (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*state[4])) + state[5];
   out_7025688067405742044[6] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*state[4])) + state[6];
   out_7025688067405742044[7] = state[7];
   out_7025688067405742044[8] = state[8];
}
void F_fun(double *state, double dt, double *out_8992610323869343845) {
   out_8992610323869343845[0] = 1;
   out_8992610323869343845[1] = 0;
   out_8992610323869343845[2] = 0;
   out_8992610323869343845[3] = 0;
   out_8992610323869343845[4] = 0;
   out_8992610323869343845[5] = 0;
   out_8992610323869343845[6] = 0;
   out_8992610323869343845[7] = 0;
   out_8992610323869343845[8] = 0;
   out_8992610323869343845[9] = 0;
   out_8992610323869343845[10] = 1;
   out_8992610323869343845[11] = 0;
   out_8992610323869343845[12] = 0;
   out_8992610323869343845[13] = 0;
   out_8992610323869343845[14] = 0;
   out_8992610323869343845[15] = 0;
   out_8992610323869343845[16] = 0;
   out_8992610323869343845[17] = 0;
   out_8992610323869343845[18] = 0;
   out_8992610323869343845[19] = 0;
   out_8992610323869343845[20] = 1;
   out_8992610323869343845[21] = 0;
   out_8992610323869343845[22] = 0;
   out_8992610323869343845[23] = 0;
   out_8992610323869343845[24] = 0;
   out_8992610323869343845[25] = 0;
   out_8992610323869343845[26] = 0;
   out_8992610323869343845[27] = 0;
   out_8992610323869343845[28] = 0;
   out_8992610323869343845[29] = 0;
   out_8992610323869343845[30] = 1;
   out_8992610323869343845[31] = 0;
   out_8992610323869343845[32] = 0;
   out_8992610323869343845[33] = 0;
   out_8992610323869343845[34] = 0;
   out_8992610323869343845[35] = 0;
   out_8992610323869343845[36] = 0;
   out_8992610323869343845[37] = 0;
   out_8992610323869343845[38] = 0;
   out_8992610323869343845[39] = 0;
   out_8992610323869343845[40] = 1;
   out_8992610323869343845[41] = 0;
   out_8992610323869343845[42] = 0;
   out_8992610323869343845[43] = 0;
   out_8992610323869343845[44] = 0;
   out_8992610323869343845[45] = dt*(stiffness_front*(-state[2] - state[3] + state[7])/(mass*state[1]) + (-stiffness_front - stiffness_rear)*state[5]/(mass*state[4]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[6]/(mass*state[4]));
   out_8992610323869343845[46] = -dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*pow(state[1], 2));
   out_8992610323869343845[47] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_8992610323869343845[48] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_8992610323869343845[49] = dt*((-1 - (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*pow(state[4], 2)))*state[6] - (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*pow(state[4], 2)));
   out_8992610323869343845[50] = dt*(-stiffness_front*state[0] - stiffness_rear*state[0])/(mass*state[4]) + 1;
   out_8992610323869343845[51] = dt*(-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]));
   out_8992610323869343845[52] = dt*stiffness_front*state[0]/(mass*state[1]);
   out_8992610323869343845[53] = -9.8100000000000005*dt;
   out_8992610323869343845[54] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front - pow(center_to_rear, 2)*stiffness_rear)*state[6]/(rotational_inertia*state[4]));
   out_8992610323869343845[55] = -center_to_front*dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*pow(state[1], 2));
   out_8992610323869343845[56] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_8992610323869343845[57] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_8992610323869343845[58] = dt*(-(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*pow(state[4], 2)) - (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*pow(state[4], 2)));
   out_8992610323869343845[59] = dt*(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(rotational_inertia*state[4]);
   out_8992610323869343845[60] = dt*(-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])/(rotational_inertia*state[4]) + 1;
   out_8992610323869343845[61] = center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_8992610323869343845[62] = 0;
   out_8992610323869343845[63] = 0;
   out_8992610323869343845[64] = 0;
   out_8992610323869343845[65] = 0;
   out_8992610323869343845[66] = 0;
   out_8992610323869343845[67] = 0;
   out_8992610323869343845[68] = 0;
   out_8992610323869343845[69] = 0;
   out_8992610323869343845[70] = 1;
   out_8992610323869343845[71] = 0;
   out_8992610323869343845[72] = 0;
   out_8992610323869343845[73] = 0;
   out_8992610323869343845[74] = 0;
   out_8992610323869343845[75] = 0;
   out_8992610323869343845[76] = 0;
   out_8992610323869343845[77] = 0;
   out_8992610323869343845[78] = 0;
   out_8992610323869343845[79] = 0;
   out_8992610323869343845[80] = 1;
}
void h_25(double *state, double *unused, double *out_659508963430133761) {
   out_659508963430133761[0] = state[6];
}
void H_25(double *state, double *unused, double *out_5411732216437484507) {
   out_5411732216437484507[0] = 0;
   out_5411732216437484507[1] = 0;
   out_5411732216437484507[2] = 0;
   out_5411732216437484507[3] = 0;
   out_5411732216437484507[4] = 0;
   out_5411732216437484507[5] = 0;
   out_5411732216437484507[6] = 1;
   out_5411732216437484507[7] = 0;
   out_5411732216437484507[8] = 0;
}
void h_24(double *state, double *unused, double *out_2182025158618031287) {
   out_2182025158618031287[0] = state[4];
   out_2182025158618031287[1] = state[5];
}
void H_24(double *state, double *unused, double *out_6824760464475893344) {
   out_6824760464475893344[0] = 0;
   out_6824760464475893344[1] = 0;
   out_6824760464475893344[2] = 0;
   out_6824760464475893344[3] = 0;
   out_6824760464475893344[4] = 1;
   out_6824760464475893344[5] = 0;
   out_6824760464475893344[6] = 0;
   out_6824760464475893344[7] = 0;
   out_6824760464475893344[8] = 0;
   out_6824760464475893344[9] = 0;
   out_6824760464475893344[10] = 0;
   out_6824760464475893344[11] = 0;
   out_6824760464475893344[12] = 0;
   out_6824760464475893344[13] = 0;
   out_6824760464475893344[14] = 1;
   out_6824760464475893344[15] = 0;
   out_6824760464475893344[16] = 0;
   out_6824760464475893344[17] = 0;
}
void h_30(double *state, double *unused, double *out_3607234953785618722) {
   out_3607234953785618722[0] = state[4];
}
void H_30(double *state, double *unused, double *out_7930065174944733134) {
   out_7930065174944733134[0] = 0;
   out_7930065174944733134[1] = 0;
   out_7930065174944733134[2] = 0;
   out_7930065174944733134[3] = 0;
   out_7930065174944733134[4] = 1;
   out_7930065174944733134[5] = 0;
   out_7930065174944733134[6] = 0;
   out_7930065174944733134[7] = 0;
   out_7930065174944733134[8] = 0;
}
void h_26(double *state, double *unused, double *out_6419787057055472105) {
   out_6419787057055472105[0] = state[7];
}
void H_26(double *state, double *unused, double *out_1670228897563428283) {
   out_1670228897563428283[0] = 0;
   out_1670228897563428283[1] = 0;
   out_1670228897563428283[2] = 0;
   out_1670228897563428283[3] = 0;
   out_1670228897563428283[4] = 0;
   out_1670228897563428283[5] = 0;
   out_1670228897563428283[6] = 0;
   out_1670228897563428283[7] = 1;
   out_1670228897563428283[8] = 0;
}
void h_27(double *state, double *unused, double *out_1197958375544545365) {
   out_1197958375544545365[0] = state[3];
}
void H_27(double *state, double *unused, double *out_5755301863144308223) {
   out_5755301863144308223[0] = 0;
   out_5755301863144308223[1] = 0;
   out_5755301863144308223[2] = 0;
   out_5755301863144308223[3] = 1;
   out_5755301863144308223[4] = 0;
   out_5755301863144308223[5] = 0;
   out_5755301863144308223[6] = 0;
   out_5755301863144308223[7] = 0;
   out_5755301863144308223[8] = 0;
}
void h_29(double *state, double *unused, double *out_8608908221446464367) {
   out_8608908221446464367[0] = state[1];
}
void H_29(double *state, double *unused, double *out_8440296519259125318) {
   out_8440296519259125318[0] = 0;
   out_8440296519259125318[1] = 1;
   out_8440296519259125318[2] = 0;
   out_8440296519259125318[3] = 0;
   out_8440296519259125318[4] = 0;
   out_8440296519259125318[5] = 0;
   out_8440296519259125318[6] = 0;
   out_8440296519259125318[7] = 0;
   out_8440296519259125318[8] = 0;
}
void h_28(double *state, double *unused, double *out_7519878544324146961) {
   out_7519878544324146961[0] = state[0];
}
void H_28(double *state, double *unused, double *out_3357897502189594744) {
   out_3357897502189594744[0] = 1;
   out_3357897502189594744[1] = 0;
   out_3357897502189594744[2] = 0;
   out_3357897502189594744[3] = 0;
   out_3357897502189594744[4] = 0;
   out_3357897502189594744[5] = 0;
   out_3357897502189594744[6] = 0;
   out_3357897502189594744[7] = 0;
   out_3357897502189594744[8] = 0;
}
void h_31(double *state, double *unused, double *out_7221763687323915424) {
   out_7221763687323915424[0] = state[8];
}
void H_31(double *state, double *unused, double *out_1044020795330076807) {
   out_1044020795330076807[0] = 0;
   out_1044020795330076807[1] = 0;
   out_1044020795330076807[2] = 0;
   out_1044020795330076807[3] = 0;
   out_1044020795330076807[4] = 0;
   out_1044020795330076807[5] = 0;
   out_1044020795330076807[6] = 0;
   out_1044020795330076807[7] = 0;
   out_1044020795330076807[8] = 1;
}
#include <eigen3/Eigen/Dense>
#include <iostream>

typedef Eigen::Matrix<double, DIM, DIM, Eigen::RowMajor> DDM;
typedef Eigen::Matrix<double, EDIM, EDIM, Eigen::RowMajor> EEM;
typedef Eigen::Matrix<double, DIM, EDIM, Eigen::RowMajor> DEM;

void predict(double *in_x, double *in_P, double *in_Q, double dt) {
  typedef Eigen::Matrix<double, MEDIM, MEDIM, Eigen::RowMajor> RRM;

  double nx[DIM] = {0};
  double in_F[EDIM*EDIM] = {0};

  // functions from sympy
  f_fun(in_x, dt, nx);
  F_fun(in_x, dt, in_F);


  EEM F(in_F);
  EEM P(in_P);
  EEM Q(in_Q);

  RRM F_main = F.topLeftCorner(MEDIM, MEDIM);
  P.topLeftCorner(MEDIM, MEDIM) = (F_main * P.topLeftCorner(MEDIM, MEDIM)) * F_main.transpose();
  P.topRightCorner(MEDIM, EDIM - MEDIM) = F_main * P.topRightCorner(MEDIM, EDIM - MEDIM);
  P.bottomLeftCorner(EDIM - MEDIM, MEDIM) = P.bottomLeftCorner(EDIM - MEDIM, MEDIM) * F_main.transpose();

  P = P + dt*Q;

  // copy out state
  memcpy(in_x, nx, DIM * sizeof(double));
  memcpy(in_P, P.data(), EDIM * EDIM * sizeof(double));
}

// note: extra_args dim only correct when null space projecting
// otherwise 1
template <int ZDIM, int EADIM, bool MAHA_TEST>
void update(double *in_x, double *in_P, Hfun h_fun, Hfun H_fun, Hfun Hea_fun, double *in_z, double *in_R, double *in_ea, double MAHA_THRESHOLD) {
  typedef Eigen::Matrix<double, ZDIM, ZDIM, Eigen::RowMajor> ZZM;
  typedef Eigen::Matrix<double, ZDIM, DIM, Eigen::RowMajor> ZDM;
  typedef Eigen::Matrix<double, Eigen::Dynamic, EDIM, Eigen::RowMajor> XEM;
  //typedef Eigen::Matrix<double, EDIM, ZDIM, Eigen::RowMajor> EZM;
  typedef Eigen::Matrix<double, Eigen::Dynamic, 1> X1M;
  typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> XXM;

  double in_hx[ZDIM] = {0};
  double in_H[ZDIM * DIM] = {0};
  double in_H_mod[EDIM * DIM] = {0};
  double delta_x[EDIM] = {0};
  double x_new[DIM] = {0};


  // state x, P
  Eigen::Matrix<double, ZDIM, 1> z(in_z);
  EEM P(in_P);
  ZZM pre_R(in_R);

  // functions from sympy
  h_fun(in_x, in_ea, in_hx);
  H_fun(in_x, in_ea, in_H);
  ZDM pre_H(in_H);

  // get y (y = z - hx)
  Eigen::Matrix<double, ZDIM, 1> pre_y(in_hx); pre_y = z - pre_y;
  X1M y; XXM H; XXM R;
  if (Hea_fun){
    typedef Eigen::Matrix<double, ZDIM, EADIM, Eigen::RowMajor> ZAM;
    double in_Hea[ZDIM * EADIM] = {0};
    Hea_fun(in_x, in_ea, in_Hea);
    ZAM Hea(in_Hea);
    XXM A = Hea.transpose().fullPivLu().kernel();


    y = A.transpose() * pre_y;
    H = A.transpose() * pre_H;
    R = A.transpose() * pre_R * A;
  } else {
    y = pre_y;
    H = pre_H;
    R = pre_R;
  }
  // get modified H
  H_mod_fun(in_x, in_H_mod);
  DEM H_mod(in_H_mod);
  XEM H_err = H * H_mod;

  // Do mahalobis distance test
  if (MAHA_TEST){
    XXM a = (H_err * P * H_err.transpose() + R).inverse();
    double maha_dist = y.transpose() * a * y;
    if (maha_dist > MAHA_THRESHOLD){
      R = 1.0e16 * R;
    }
  }

  // Outlier resilient weighting
  double weight = 1;//(1.5)/(1 + y.squaredNorm()/R.sum());

  // kalman gains and I_KH
  XXM S = ((H_err * P) * H_err.transpose()) + R/weight;
  XEM KT = S.fullPivLu().solve(H_err * P.transpose());
  //EZM K = KT.transpose(); TODO: WHY DOES THIS NOT COMPILE?
  //EZM K = S.fullPivLu().solve(H_err * P.transpose()).transpose();
  //std::cout << "Here is the matrix rot:\n" << K << std::endl;
  EEM I_KH = Eigen::Matrix<double, EDIM, EDIM>::Identity() - (KT.transpose() * H_err);

  // update state by injecting dx
  Eigen::Matrix<double, EDIM, 1> dx(delta_x);
  dx  = (KT.transpose() * y);
  memcpy(delta_x, dx.data(), EDIM * sizeof(double));
  err_fun(in_x, delta_x, x_new);
  Eigen::Matrix<double, DIM, 1> x(x_new);

  // update cov
  P = ((I_KH * P) * I_KH.transpose()) + ((KT.transpose() * R) * KT);

  // copy out state
  memcpy(in_x, x.data(), DIM * sizeof(double));
  memcpy(in_P, P.data(), EDIM * EDIM * sizeof(double));
  memcpy(in_z, y.data(), y.rows() * sizeof(double));
}




}
extern "C" {

void car_update_25(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_25, H_25, NULL, in_z, in_R, in_ea, MAHA_THRESH_25);
}
void car_update_24(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<2, 3, 0>(in_x, in_P, h_24, H_24, NULL, in_z, in_R, in_ea, MAHA_THRESH_24);
}
void car_update_30(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_30, H_30, NULL, in_z, in_R, in_ea, MAHA_THRESH_30);
}
void car_update_26(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_26, H_26, NULL, in_z, in_R, in_ea, MAHA_THRESH_26);
}
void car_update_27(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_27, H_27, NULL, in_z, in_R, in_ea, MAHA_THRESH_27);
}
void car_update_29(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_29, H_29, NULL, in_z, in_R, in_ea, MAHA_THRESH_29);
}
void car_update_28(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_28, H_28, NULL, in_z, in_R, in_ea, MAHA_THRESH_28);
}
void car_update_31(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_31, H_31, NULL, in_z, in_R, in_ea, MAHA_THRESH_31);
}
void car_err_fun(double *nom_x, double *delta_x, double *out_4898271858899960872) {
  err_fun(nom_x, delta_x, out_4898271858899960872);
}
void car_inv_err_fun(double *nom_x, double *true_x, double *out_6923660614592572242) {
  inv_err_fun(nom_x, true_x, out_6923660614592572242);
}
void car_H_mod_fun(double *state, double *out_2317713943645087999) {
  H_mod_fun(state, out_2317713943645087999);
}
void car_f_fun(double *state, double dt, double *out_7025688067405742044) {
  f_fun(state,  dt, out_7025688067405742044);
}
void car_F_fun(double *state, double dt, double *out_8992610323869343845) {
  F_fun(state,  dt, out_8992610323869343845);
}
void car_h_25(double *state, double *unused, double *out_659508963430133761) {
  h_25(state, unused, out_659508963430133761);
}
void car_H_25(double *state, double *unused, double *out_5411732216437484507) {
  H_25(state, unused, out_5411732216437484507);
}
void car_h_24(double *state, double *unused, double *out_2182025158618031287) {
  h_24(state, unused, out_2182025158618031287);
}
void car_H_24(double *state, double *unused, double *out_6824760464475893344) {
  H_24(state, unused, out_6824760464475893344);
}
void car_h_30(double *state, double *unused, double *out_3607234953785618722) {
  h_30(state, unused, out_3607234953785618722);
}
void car_H_30(double *state, double *unused, double *out_7930065174944733134) {
  H_30(state, unused, out_7930065174944733134);
}
void car_h_26(double *state, double *unused, double *out_6419787057055472105) {
  h_26(state, unused, out_6419787057055472105);
}
void car_H_26(double *state, double *unused, double *out_1670228897563428283) {
  H_26(state, unused, out_1670228897563428283);
}
void car_h_27(double *state, double *unused, double *out_1197958375544545365) {
  h_27(state, unused, out_1197958375544545365);
}
void car_H_27(double *state, double *unused, double *out_5755301863144308223) {
  H_27(state, unused, out_5755301863144308223);
}
void car_h_29(double *state, double *unused, double *out_8608908221446464367) {
  h_29(state, unused, out_8608908221446464367);
}
void car_H_29(double *state, double *unused, double *out_8440296519259125318) {
  H_29(state, unused, out_8440296519259125318);
}
void car_h_28(double *state, double *unused, double *out_7519878544324146961) {
  h_28(state, unused, out_7519878544324146961);
}
void car_H_28(double *state, double *unused, double *out_3357897502189594744) {
  H_28(state, unused, out_3357897502189594744);
}
void car_h_31(double *state, double *unused, double *out_7221763687323915424) {
  h_31(state, unused, out_7221763687323915424);
}
void car_H_31(double *state, double *unused, double *out_1044020795330076807) {
  H_31(state, unused, out_1044020795330076807);
}
void car_predict(double *in_x, double *in_P, double *in_Q, double dt) {
  predict(in_x, in_P, in_Q, dt);
}
void car_set_mass(double x) {
  set_mass(x);
}
void car_set_rotational_inertia(double x) {
  set_rotational_inertia(x);
}
void car_set_center_to_front(double x) {
  set_center_to_front(x);
}
void car_set_center_to_rear(double x) {
  set_center_to_rear(x);
}
void car_set_stiffness_front(double x) {
  set_stiffness_front(x);
}
void car_set_stiffness_rear(double x) {
  set_stiffness_rear(x);
}
}

const EKF car = {
  .name = "car",
  .kinds = { 25, 24, 30, 26, 27, 29, 28, 31 },
  .feature_kinds = {  },
  .f_fun = car_f_fun,
  .F_fun = car_F_fun,
  .err_fun = car_err_fun,
  .inv_err_fun = car_inv_err_fun,
  .H_mod_fun = car_H_mod_fun,
  .predict = car_predict,
  .hs = {
    { 25, car_h_25 },
    { 24, car_h_24 },
    { 30, car_h_30 },
    { 26, car_h_26 },
    { 27, car_h_27 },
    { 29, car_h_29 },
    { 28, car_h_28 },
    { 31, car_h_31 },
  },
  .Hs = {
    { 25, car_H_25 },
    { 24, car_H_24 },
    { 30, car_H_30 },
    { 26, car_H_26 },
    { 27, car_H_27 },
    { 29, car_H_29 },
    { 28, car_H_28 },
    { 31, car_H_31 },
  },
  .updates = {
    { 25, car_update_25 },
    { 24, car_update_24 },
    { 30, car_update_30 },
    { 26, car_update_26 },
    { 27, car_update_27 },
    { 29, car_update_29 },
    { 28, car_update_28 },
    { 31, car_update_31 },
  },
  .Hes = {
  },
  .sets = {
    { "mass", car_set_mass },
    { "rotational_inertia", car_set_rotational_inertia },
    { "center_to_front", car_set_center_to_front },
    { "center_to_rear", car_set_center_to_rear },
    { "stiffness_front", car_set_stiffness_front },
    { "stiffness_rear", car_set_stiffness_rear },
  },
  .extra_routines = {
  },
};

ekf_lib_init(car)
