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
void err_fun(double *nom_x, double *delta_x, double *out_151033249954379814) {
   out_151033249954379814[0] = delta_x[0] + nom_x[0];
   out_151033249954379814[1] = delta_x[1] + nom_x[1];
   out_151033249954379814[2] = delta_x[2] + nom_x[2];
   out_151033249954379814[3] = delta_x[3] + nom_x[3];
   out_151033249954379814[4] = delta_x[4] + nom_x[4];
   out_151033249954379814[5] = delta_x[5] + nom_x[5];
   out_151033249954379814[6] = delta_x[6] + nom_x[6];
   out_151033249954379814[7] = delta_x[7] + nom_x[7];
   out_151033249954379814[8] = delta_x[8] + nom_x[8];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_183614081703202987) {
   out_183614081703202987[0] = -nom_x[0] + true_x[0];
   out_183614081703202987[1] = -nom_x[1] + true_x[1];
   out_183614081703202987[2] = -nom_x[2] + true_x[2];
   out_183614081703202987[3] = -nom_x[3] + true_x[3];
   out_183614081703202987[4] = -nom_x[4] + true_x[4];
   out_183614081703202987[5] = -nom_x[5] + true_x[5];
   out_183614081703202987[6] = -nom_x[6] + true_x[6];
   out_183614081703202987[7] = -nom_x[7] + true_x[7];
   out_183614081703202987[8] = -nom_x[8] + true_x[8];
}
void H_mod_fun(double *state, double *out_7146229962937001843) {
   out_7146229962937001843[0] = 1.0;
   out_7146229962937001843[1] = 0.0;
   out_7146229962937001843[2] = 0.0;
   out_7146229962937001843[3] = 0.0;
   out_7146229962937001843[4] = 0.0;
   out_7146229962937001843[5] = 0.0;
   out_7146229962937001843[6] = 0.0;
   out_7146229962937001843[7] = 0.0;
   out_7146229962937001843[8] = 0.0;
   out_7146229962937001843[9] = 0.0;
   out_7146229962937001843[10] = 1.0;
   out_7146229962937001843[11] = 0.0;
   out_7146229962937001843[12] = 0.0;
   out_7146229962937001843[13] = 0.0;
   out_7146229962937001843[14] = 0.0;
   out_7146229962937001843[15] = 0.0;
   out_7146229962937001843[16] = 0.0;
   out_7146229962937001843[17] = 0.0;
   out_7146229962937001843[18] = 0.0;
   out_7146229962937001843[19] = 0.0;
   out_7146229962937001843[20] = 1.0;
   out_7146229962937001843[21] = 0.0;
   out_7146229962937001843[22] = 0.0;
   out_7146229962937001843[23] = 0.0;
   out_7146229962937001843[24] = 0.0;
   out_7146229962937001843[25] = 0.0;
   out_7146229962937001843[26] = 0.0;
   out_7146229962937001843[27] = 0.0;
   out_7146229962937001843[28] = 0.0;
   out_7146229962937001843[29] = 0.0;
   out_7146229962937001843[30] = 1.0;
   out_7146229962937001843[31] = 0.0;
   out_7146229962937001843[32] = 0.0;
   out_7146229962937001843[33] = 0.0;
   out_7146229962937001843[34] = 0.0;
   out_7146229962937001843[35] = 0.0;
   out_7146229962937001843[36] = 0.0;
   out_7146229962937001843[37] = 0.0;
   out_7146229962937001843[38] = 0.0;
   out_7146229962937001843[39] = 0.0;
   out_7146229962937001843[40] = 1.0;
   out_7146229962937001843[41] = 0.0;
   out_7146229962937001843[42] = 0.0;
   out_7146229962937001843[43] = 0.0;
   out_7146229962937001843[44] = 0.0;
   out_7146229962937001843[45] = 0.0;
   out_7146229962937001843[46] = 0.0;
   out_7146229962937001843[47] = 0.0;
   out_7146229962937001843[48] = 0.0;
   out_7146229962937001843[49] = 0.0;
   out_7146229962937001843[50] = 1.0;
   out_7146229962937001843[51] = 0.0;
   out_7146229962937001843[52] = 0.0;
   out_7146229962937001843[53] = 0.0;
   out_7146229962937001843[54] = 0.0;
   out_7146229962937001843[55] = 0.0;
   out_7146229962937001843[56] = 0.0;
   out_7146229962937001843[57] = 0.0;
   out_7146229962937001843[58] = 0.0;
   out_7146229962937001843[59] = 0.0;
   out_7146229962937001843[60] = 1.0;
   out_7146229962937001843[61] = 0.0;
   out_7146229962937001843[62] = 0.0;
   out_7146229962937001843[63] = 0.0;
   out_7146229962937001843[64] = 0.0;
   out_7146229962937001843[65] = 0.0;
   out_7146229962937001843[66] = 0.0;
   out_7146229962937001843[67] = 0.0;
   out_7146229962937001843[68] = 0.0;
   out_7146229962937001843[69] = 0.0;
   out_7146229962937001843[70] = 1.0;
   out_7146229962937001843[71] = 0.0;
   out_7146229962937001843[72] = 0.0;
   out_7146229962937001843[73] = 0.0;
   out_7146229962937001843[74] = 0.0;
   out_7146229962937001843[75] = 0.0;
   out_7146229962937001843[76] = 0.0;
   out_7146229962937001843[77] = 0.0;
   out_7146229962937001843[78] = 0.0;
   out_7146229962937001843[79] = 0.0;
   out_7146229962937001843[80] = 1.0;
}
void f_fun(double *state, double dt, double *out_8130334584715691889) {
   out_8130334584715691889[0] = state[0];
   out_8130334584715691889[1] = state[1];
   out_8130334584715691889[2] = state[2];
   out_8130334584715691889[3] = state[3];
   out_8130334584715691889[4] = state[4];
   out_8130334584715691889[5] = dt*((-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]))*state[6] - 9.8000000000000007*state[8] + stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*state[1]) + (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*state[4])) + state[5];
   out_8130334584715691889[6] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*state[4])) + state[6];
   out_8130334584715691889[7] = state[7];
   out_8130334584715691889[8] = state[8];
}
void F_fun(double *state, double dt, double *out_2461222714691540299) {
   out_2461222714691540299[0] = 1;
   out_2461222714691540299[1] = 0;
   out_2461222714691540299[2] = 0;
   out_2461222714691540299[3] = 0;
   out_2461222714691540299[4] = 0;
   out_2461222714691540299[5] = 0;
   out_2461222714691540299[6] = 0;
   out_2461222714691540299[7] = 0;
   out_2461222714691540299[8] = 0;
   out_2461222714691540299[9] = 0;
   out_2461222714691540299[10] = 1;
   out_2461222714691540299[11] = 0;
   out_2461222714691540299[12] = 0;
   out_2461222714691540299[13] = 0;
   out_2461222714691540299[14] = 0;
   out_2461222714691540299[15] = 0;
   out_2461222714691540299[16] = 0;
   out_2461222714691540299[17] = 0;
   out_2461222714691540299[18] = 0;
   out_2461222714691540299[19] = 0;
   out_2461222714691540299[20] = 1;
   out_2461222714691540299[21] = 0;
   out_2461222714691540299[22] = 0;
   out_2461222714691540299[23] = 0;
   out_2461222714691540299[24] = 0;
   out_2461222714691540299[25] = 0;
   out_2461222714691540299[26] = 0;
   out_2461222714691540299[27] = 0;
   out_2461222714691540299[28] = 0;
   out_2461222714691540299[29] = 0;
   out_2461222714691540299[30] = 1;
   out_2461222714691540299[31] = 0;
   out_2461222714691540299[32] = 0;
   out_2461222714691540299[33] = 0;
   out_2461222714691540299[34] = 0;
   out_2461222714691540299[35] = 0;
   out_2461222714691540299[36] = 0;
   out_2461222714691540299[37] = 0;
   out_2461222714691540299[38] = 0;
   out_2461222714691540299[39] = 0;
   out_2461222714691540299[40] = 1;
   out_2461222714691540299[41] = 0;
   out_2461222714691540299[42] = 0;
   out_2461222714691540299[43] = 0;
   out_2461222714691540299[44] = 0;
   out_2461222714691540299[45] = dt*(stiffness_front*(-state[2] - state[3] + state[7])/(mass*state[1]) + (-stiffness_front - stiffness_rear)*state[5]/(mass*state[4]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[6]/(mass*state[4]));
   out_2461222714691540299[46] = -dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*pow(state[1], 2));
   out_2461222714691540299[47] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_2461222714691540299[48] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_2461222714691540299[49] = dt*((-1 - (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*pow(state[4], 2)))*state[6] - (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*pow(state[4], 2)));
   out_2461222714691540299[50] = dt*(-stiffness_front*state[0] - stiffness_rear*state[0])/(mass*state[4]) + 1;
   out_2461222714691540299[51] = dt*(-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]));
   out_2461222714691540299[52] = dt*stiffness_front*state[0]/(mass*state[1]);
   out_2461222714691540299[53] = -9.8000000000000007*dt;
   out_2461222714691540299[54] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front - pow(center_to_rear, 2)*stiffness_rear)*state[6]/(rotational_inertia*state[4]));
   out_2461222714691540299[55] = -center_to_front*dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*pow(state[1], 2));
   out_2461222714691540299[56] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_2461222714691540299[57] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_2461222714691540299[58] = dt*(-(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*pow(state[4], 2)) - (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*pow(state[4], 2)));
   out_2461222714691540299[59] = dt*(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(rotational_inertia*state[4]);
   out_2461222714691540299[60] = dt*(-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])/(rotational_inertia*state[4]) + 1;
   out_2461222714691540299[61] = center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_2461222714691540299[62] = 0;
   out_2461222714691540299[63] = 0;
   out_2461222714691540299[64] = 0;
   out_2461222714691540299[65] = 0;
   out_2461222714691540299[66] = 0;
   out_2461222714691540299[67] = 0;
   out_2461222714691540299[68] = 0;
   out_2461222714691540299[69] = 0;
   out_2461222714691540299[70] = 1;
   out_2461222714691540299[71] = 0;
   out_2461222714691540299[72] = 0;
   out_2461222714691540299[73] = 0;
   out_2461222714691540299[74] = 0;
   out_2461222714691540299[75] = 0;
   out_2461222714691540299[76] = 0;
   out_2461222714691540299[77] = 0;
   out_2461222714691540299[78] = 0;
   out_2461222714691540299[79] = 0;
   out_2461222714691540299[80] = 1;
}
void h_25(double *state, double *unused, double *out_6813335027708603723) {
   out_6813335027708603723[0] = state[6];
}
void H_25(double *state, double *unused, double *out_2046958262975235394) {
   out_2046958262975235394[0] = 0;
   out_2046958262975235394[1] = 0;
   out_2046958262975235394[2] = 0;
   out_2046958262975235394[3] = 0;
   out_2046958262975235394[4] = 0;
   out_2046958262975235394[5] = 0;
   out_2046958262975235394[6] = 1;
   out_2046958262975235394[7] = 0;
   out_2046958262975235394[8] = 0;
}
void h_24(double *state, double *unused, double *out_2151628798375541770) {
   out_2151628798375541770[0] = state[4];
   out_2151628798375541770[1] = state[5];
}
void H_24(double *state, double *unused, double *out_6793467589941744428) {
   out_6793467589941744428[0] = 0;
   out_6793467589941744428[1] = 0;
   out_6793467589941744428[2] = 0;
   out_6793467589941744428[3] = 0;
   out_6793467589941744428[4] = 1;
   out_6793467589941744428[5] = 0;
   out_6793467589941744428[6] = 0;
   out_6793467589941744428[7] = 0;
   out_6793467589941744428[8] = 0;
   out_6793467589941744428[9] = 0;
   out_6793467589941744428[10] = 0;
   out_6793467589941744428[11] = 0;
   out_6793467589941744428[12] = 0;
   out_6793467589941744428[13] = 0;
   out_6793467589941744428[14] = 1;
   out_6793467589941744428[15] = 0;
   out_6793467589941744428[16] = 0;
   out_6793467589941744428[17] = 0;
}
void h_30(double *state, double *unused, double *out_7995959517746925543) {
   out_7995959517746925543[0] = state[4];
}
void H_30(double *state, double *unused, double *out_4869732078516381361) {
   out_4869732078516381361[0] = 0;
   out_4869732078516381361[1] = 0;
   out_4869732078516381361[2] = 0;
   out_4869732078516381361[3] = 0;
   out_4869732078516381361[4] = 1;
   out_4869732078516381361[5] = 0;
   out_4869732078516381361[6] = 0;
   out_4869732078516381361[7] = 0;
   out_4869732078516381361[8] = 0;
}
void h_26(double *state, double *unused, double *out_7052449664845263108) {
   out_7052449664845263108[0] = state[7];
}
void H_26(double *state, double *unused, double *out_5788461581849291618) {
   out_5788461581849291618[0] = 0;
   out_5788461581849291618[1] = 0;
   out_5788461581849291618[2] = 0;
   out_5788461581849291618[3] = 0;
   out_5788461581849291618[4] = 0;
   out_5788461581849291618[5] = 0;
   out_5788461581849291618[6] = 0;
   out_5788461581849291618[7] = 1;
   out_5788461581849291618[8] = 0;
}
void h_27(double *state, double *unused, double *out_3776548976264271038) {
   out_3776548976264271038[0] = state[3];
}
void H_27(double *state, double *unused, double *out_2694968766715956450) {
   out_2694968766715956450[0] = 0;
   out_2694968766715956450[1] = 0;
   out_2694968766715956450[2] = 0;
   out_2694968766715956450[3] = 1;
   out_2694968766715956450[4] = 0;
   out_2694968766715956450[5] = 0;
   out_2694968766715956450[6] = 0;
   out_2694968766715956450[7] = 0;
   out_2694968766715956450[8] = 0;
}
void h_29(double *state, double *unused, double *out_2994286250086079898) {
   out_2994286250086079898[0] = state[1];
}
void H_29(double *state, double *unused, double *out_981606039846405417) {
   out_981606039846405417[0] = 0;
   out_981606039846405417[1] = 1;
   out_981606039846405417[2] = 0;
   out_981606039846405417[3] = 0;
   out_981606039846405417[4] = 0;
   out_981606039846405417[5] = 0;
   out_981606039846405417[6] = 0;
   out_981606039846405417[7] = 0;
   out_981606039846405417[8] = 0;
}
void h_28(double *state, double *unused, double *out_8174501057545900050) {
   out_8174501057545900050[0] = state[0];
}
void H_28(double *state, double *unused, double *out_4100792977223125157) {
   out_4100792977223125157[0] = 1;
   out_4100792977223125157[1] = 0;
   out_4100792977223125157[2] = 0;
   out_4100792977223125157[3] = 0;
   out_4100792977223125157[4] = 0;
   out_4100792977223125157[5] = 0;
   out_4100792977223125157[6] = 0;
   out_4100792977223125157[7] = 0;
   out_4100792977223125157[8] = 0;
}
void h_31(double *state, double *unused, double *out_378214085146908073) {
   out_378214085146908073[0] = state[8];
}
void H_31(double *state, double *unused, double *out_2016312301098274966) {
   out_2016312301098274966[0] = 0;
   out_2016312301098274966[1] = 0;
   out_2016312301098274966[2] = 0;
   out_2016312301098274966[3] = 0;
   out_2016312301098274966[4] = 0;
   out_2016312301098274966[5] = 0;
   out_2016312301098274966[6] = 0;
   out_2016312301098274966[7] = 0;
   out_2016312301098274966[8] = 1;
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
void car_err_fun(double *nom_x, double *delta_x, double *out_151033249954379814) {
  err_fun(nom_x, delta_x, out_151033249954379814);
}
void car_inv_err_fun(double *nom_x, double *true_x, double *out_183614081703202987) {
  inv_err_fun(nom_x, true_x, out_183614081703202987);
}
void car_H_mod_fun(double *state, double *out_7146229962937001843) {
  H_mod_fun(state, out_7146229962937001843);
}
void car_f_fun(double *state, double dt, double *out_8130334584715691889) {
  f_fun(state,  dt, out_8130334584715691889);
}
void car_F_fun(double *state, double dt, double *out_2461222714691540299) {
  F_fun(state,  dt, out_2461222714691540299);
}
void car_h_25(double *state, double *unused, double *out_6813335027708603723) {
  h_25(state, unused, out_6813335027708603723);
}
void car_H_25(double *state, double *unused, double *out_2046958262975235394) {
  H_25(state, unused, out_2046958262975235394);
}
void car_h_24(double *state, double *unused, double *out_2151628798375541770) {
  h_24(state, unused, out_2151628798375541770);
}
void car_H_24(double *state, double *unused, double *out_6793467589941744428) {
  H_24(state, unused, out_6793467589941744428);
}
void car_h_30(double *state, double *unused, double *out_7995959517746925543) {
  h_30(state, unused, out_7995959517746925543);
}
void car_H_30(double *state, double *unused, double *out_4869732078516381361) {
  H_30(state, unused, out_4869732078516381361);
}
void car_h_26(double *state, double *unused, double *out_7052449664845263108) {
  h_26(state, unused, out_7052449664845263108);
}
void car_H_26(double *state, double *unused, double *out_5788461581849291618) {
  H_26(state, unused, out_5788461581849291618);
}
void car_h_27(double *state, double *unused, double *out_3776548976264271038) {
  h_27(state, unused, out_3776548976264271038);
}
void car_H_27(double *state, double *unused, double *out_2694968766715956450) {
  H_27(state, unused, out_2694968766715956450);
}
void car_h_29(double *state, double *unused, double *out_2994286250086079898) {
  h_29(state, unused, out_2994286250086079898);
}
void car_H_29(double *state, double *unused, double *out_981606039846405417) {
  H_29(state, unused, out_981606039846405417);
}
void car_h_28(double *state, double *unused, double *out_8174501057545900050) {
  h_28(state, unused, out_8174501057545900050);
}
void car_H_28(double *state, double *unused, double *out_4100792977223125157) {
  H_28(state, unused, out_4100792977223125157);
}
void car_h_31(double *state, double *unused, double *out_378214085146908073) {
  h_31(state, unused, out_378214085146908073);
}
void car_H_31(double *state, double *unused, double *out_2016312301098274966) {
  H_31(state, unused, out_2016312301098274966);
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
