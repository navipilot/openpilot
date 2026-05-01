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
void err_fun(double *nom_x, double *delta_x, double *out_417238876910560070) {
   out_417238876910560070[0] = delta_x[0] + nom_x[0];
   out_417238876910560070[1] = delta_x[1] + nom_x[1];
   out_417238876910560070[2] = delta_x[2] + nom_x[2];
   out_417238876910560070[3] = delta_x[3] + nom_x[3];
   out_417238876910560070[4] = delta_x[4] + nom_x[4];
   out_417238876910560070[5] = delta_x[5] + nom_x[5];
   out_417238876910560070[6] = delta_x[6] + nom_x[6];
   out_417238876910560070[7] = delta_x[7] + nom_x[7];
   out_417238876910560070[8] = delta_x[8] + nom_x[8];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_2214194102385980691) {
   out_2214194102385980691[0] = -nom_x[0] + true_x[0];
   out_2214194102385980691[1] = -nom_x[1] + true_x[1];
   out_2214194102385980691[2] = -nom_x[2] + true_x[2];
   out_2214194102385980691[3] = -nom_x[3] + true_x[3];
   out_2214194102385980691[4] = -nom_x[4] + true_x[4];
   out_2214194102385980691[5] = -nom_x[5] + true_x[5];
   out_2214194102385980691[6] = -nom_x[6] + true_x[6];
   out_2214194102385980691[7] = -nom_x[7] + true_x[7];
   out_2214194102385980691[8] = -nom_x[8] + true_x[8];
}
void H_mod_fun(double *state, double *out_2617678226180198985) {
   out_2617678226180198985[0] = 1.0;
   out_2617678226180198985[1] = 0.0;
   out_2617678226180198985[2] = 0.0;
   out_2617678226180198985[3] = 0.0;
   out_2617678226180198985[4] = 0.0;
   out_2617678226180198985[5] = 0.0;
   out_2617678226180198985[6] = 0.0;
   out_2617678226180198985[7] = 0.0;
   out_2617678226180198985[8] = 0.0;
   out_2617678226180198985[9] = 0.0;
   out_2617678226180198985[10] = 1.0;
   out_2617678226180198985[11] = 0.0;
   out_2617678226180198985[12] = 0.0;
   out_2617678226180198985[13] = 0.0;
   out_2617678226180198985[14] = 0.0;
   out_2617678226180198985[15] = 0.0;
   out_2617678226180198985[16] = 0.0;
   out_2617678226180198985[17] = 0.0;
   out_2617678226180198985[18] = 0.0;
   out_2617678226180198985[19] = 0.0;
   out_2617678226180198985[20] = 1.0;
   out_2617678226180198985[21] = 0.0;
   out_2617678226180198985[22] = 0.0;
   out_2617678226180198985[23] = 0.0;
   out_2617678226180198985[24] = 0.0;
   out_2617678226180198985[25] = 0.0;
   out_2617678226180198985[26] = 0.0;
   out_2617678226180198985[27] = 0.0;
   out_2617678226180198985[28] = 0.0;
   out_2617678226180198985[29] = 0.0;
   out_2617678226180198985[30] = 1.0;
   out_2617678226180198985[31] = 0.0;
   out_2617678226180198985[32] = 0.0;
   out_2617678226180198985[33] = 0.0;
   out_2617678226180198985[34] = 0.0;
   out_2617678226180198985[35] = 0.0;
   out_2617678226180198985[36] = 0.0;
   out_2617678226180198985[37] = 0.0;
   out_2617678226180198985[38] = 0.0;
   out_2617678226180198985[39] = 0.0;
   out_2617678226180198985[40] = 1.0;
   out_2617678226180198985[41] = 0.0;
   out_2617678226180198985[42] = 0.0;
   out_2617678226180198985[43] = 0.0;
   out_2617678226180198985[44] = 0.0;
   out_2617678226180198985[45] = 0.0;
   out_2617678226180198985[46] = 0.0;
   out_2617678226180198985[47] = 0.0;
   out_2617678226180198985[48] = 0.0;
   out_2617678226180198985[49] = 0.0;
   out_2617678226180198985[50] = 1.0;
   out_2617678226180198985[51] = 0.0;
   out_2617678226180198985[52] = 0.0;
   out_2617678226180198985[53] = 0.0;
   out_2617678226180198985[54] = 0.0;
   out_2617678226180198985[55] = 0.0;
   out_2617678226180198985[56] = 0.0;
   out_2617678226180198985[57] = 0.0;
   out_2617678226180198985[58] = 0.0;
   out_2617678226180198985[59] = 0.0;
   out_2617678226180198985[60] = 1.0;
   out_2617678226180198985[61] = 0.0;
   out_2617678226180198985[62] = 0.0;
   out_2617678226180198985[63] = 0.0;
   out_2617678226180198985[64] = 0.0;
   out_2617678226180198985[65] = 0.0;
   out_2617678226180198985[66] = 0.0;
   out_2617678226180198985[67] = 0.0;
   out_2617678226180198985[68] = 0.0;
   out_2617678226180198985[69] = 0.0;
   out_2617678226180198985[70] = 1.0;
   out_2617678226180198985[71] = 0.0;
   out_2617678226180198985[72] = 0.0;
   out_2617678226180198985[73] = 0.0;
   out_2617678226180198985[74] = 0.0;
   out_2617678226180198985[75] = 0.0;
   out_2617678226180198985[76] = 0.0;
   out_2617678226180198985[77] = 0.0;
   out_2617678226180198985[78] = 0.0;
   out_2617678226180198985[79] = 0.0;
   out_2617678226180198985[80] = 1.0;
}
void f_fun(double *state, double dt, double *out_2248561019141826344) {
   out_2248561019141826344[0] = state[0];
   out_2248561019141826344[1] = state[1];
   out_2248561019141826344[2] = state[2];
   out_2248561019141826344[3] = state[3];
   out_2248561019141826344[4] = state[4];
   out_2248561019141826344[5] = dt*((-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]))*state[6] - 9.8100000000000005*state[8] + stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*state[1]) + (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*state[4])) + state[5];
   out_2248561019141826344[6] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*state[4])) + state[6];
   out_2248561019141826344[7] = state[7];
   out_2248561019141826344[8] = state[8];
}
void F_fun(double *state, double dt, double *out_1589062328883606575) {
   out_1589062328883606575[0] = 1;
   out_1589062328883606575[1] = 0;
   out_1589062328883606575[2] = 0;
   out_1589062328883606575[3] = 0;
   out_1589062328883606575[4] = 0;
   out_1589062328883606575[5] = 0;
   out_1589062328883606575[6] = 0;
   out_1589062328883606575[7] = 0;
   out_1589062328883606575[8] = 0;
   out_1589062328883606575[9] = 0;
   out_1589062328883606575[10] = 1;
   out_1589062328883606575[11] = 0;
   out_1589062328883606575[12] = 0;
   out_1589062328883606575[13] = 0;
   out_1589062328883606575[14] = 0;
   out_1589062328883606575[15] = 0;
   out_1589062328883606575[16] = 0;
   out_1589062328883606575[17] = 0;
   out_1589062328883606575[18] = 0;
   out_1589062328883606575[19] = 0;
   out_1589062328883606575[20] = 1;
   out_1589062328883606575[21] = 0;
   out_1589062328883606575[22] = 0;
   out_1589062328883606575[23] = 0;
   out_1589062328883606575[24] = 0;
   out_1589062328883606575[25] = 0;
   out_1589062328883606575[26] = 0;
   out_1589062328883606575[27] = 0;
   out_1589062328883606575[28] = 0;
   out_1589062328883606575[29] = 0;
   out_1589062328883606575[30] = 1;
   out_1589062328883606575[31] = 0;
   out_1589062328883606575[32] = 0;
   out_1589062328883606575[33] = 0;
   out_1589062328883606575[34] = 0;
   out_1589062328883606575[35] = 0;
   out_1589062328883606575[36] = 0;
   out_1589062328883606575[37] = 0;
   out_1589062328883606575[38] = 0;
   out_1589062328883606575[39] = 0;
   out_1589062328883606575[40] = 1;
   out_1589062328883606575[41] = 0;
   out_1589062328883606575[42] = 0;
   out_1589062328883606575[43] = 0;
   out_1589062328883606575[44] = 0;
   out_1589062328883606575[45] = dt*(stiffness_front*(-state[2] - state[3] + state[7])/(mass*state[1]) + (-stiffness_front - stiffness_rear)*state[5]/(mass*state[4]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[6]/(mass*state[4]));
   out_1589062328883606575[46] = -dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*pow(state[1], 2));
   out_1589062328883606575[47] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_1589062328883606575[48] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_1589062328883606575[49] = dt*((-1 - (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*pow(state[4], 2)))*state[6] - (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*pow(state[4], 2)));
   out_1589062328883606575[50] = dt*(-stiffness_front*state[0] - stiffness_rear*state[0])/(mass*state[4]) + 1;
   out_1589062328883606575[51] = dt*(-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]));
   out_1589062328883606575[52] = dt*stiffness_front*state[0]/(mass*state[1]);
   out_1589062328883606575[53] = -9.8100000000000005*dt;
   out_1589062328883606575[54] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front - pow(center_to_rear, 2)*stiffness_rear)*state[6]/(rotational_inertia*state[4]));
   out_1589062328883606575[55] = -center_to_front*dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*pow(state[1], 2));
   out_1589062328883606575[56] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_1589062328883606575[57] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_1589062328883606575[58] = dt*(-(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*pow(state[4], 2)) - (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*pow(state[4], 2)));
   out_1589062328883606575[59] = dt*(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(rotational_inertia*state[4]);
   out_1589062328883606575[60] = dt*(-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])/(rotational_inertia*state[4]) + 1;
   out_1589062328883606575[61] = center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_1589062328883606575[62] = 0;
   out_1589062328883606575[63] = 0;
   out_1589062328883606575[64] = 0;
   out_1589062328883606575[65] = 0;
   out_1589062328883606575[66] = 0;
   out_1589062328883606575[67] = 0;
   out_1589062328883606575[68] = 0;
   out_1589062328883606575[69] = 0;
   out_1589062328883606575[70] = 1;
   out_1589062328883606575[71] = 0;
   out_1589062328883606575[72] = 0;
   out_1589062328883606575[73] = 0;
   out_1589062328883606575[74] = 0;
   out_1589062328883606575[75] = 0;
   out_1589062328883606575[76] = 0;
   out_1589062328883606575[77] = 0;
   out_1589062328883606575[78] = 0;
   out_1589062328883606575[79] = 0;
   out_1589062328883606575[80] = 1;
}
void h_25(double *state, double *unused, double *out_4251901549604751950) {
   out_4251901549604751950[0] = state[6];
}
void H_25(double *state, double *unused, double *out_7412509069108068094) {
   out_7412509069108068094[0] = 0;
   out_7412509069108068094[1] = 0;
   out_7412509069108068094[2] = 0;
   out_7412509069108068094[3] = 0;
   out_7412509069108068094[4] = 0;
   out_7412509069108068094[5] = 0;
   out_7412509069108068094[6] = 1;
   out_7412509069108068094[7] = 0;
   out_7412509069108068094[8] = 0;
}
void h_24(double *state, double *unused, double *out_6482324594163419455) {
   out_6482324594163419455[0] = state[4];
   out_6482324594163419455[1] = state[5];
}
void H_24(double *state, double *unused, double *out_1815556116961127131) {
   out_1815556116961127131[0] = 0;
   out_1815556116961127131[1] = 0;
   out_1815556116961127131[2] = 0;
   out_1815556116961127131[3] = 0;
   out_1815556116961127131[4] = 1;
   out_1815556116961127131[5] = 0;
   out_1815556116961127131[6] = 0;
   out_1815556116961127131[7] = 0;
   out_1815556116961127131[8] = 0;
   out_1815556116961127131[9] = 0;
   out_1815556116961127131[10] = 0;
   out_1815556116961127131[11] = 0;
   out_1815556116961127131[12] = 0;
   out_1815556116961127131[13] = 0;
   out_1815556116961127131[14] = 1;
   out_1815556116961127131[15] = 0;
   out_1815556116961127131[16] = 0;
   out_1815556116961127131[17] = 0;
}
void h_30(double *state, double *unused, double *out_3976707487320246061) {
   out_3976707487320246061[0] = state[4];
}
void H_30(double *state, double *unused, double *out_6506538674473875324) {
   out_6506538674473875324[0] = 0;
   out_6506538674473875324[1] = 0;
   out_6506538674473875324[2] = 0;
   out_6506538674473875324[3] = 0;
   out_6506538674473875324[4] = 1;
   out_6506538674473875324[5] = 0;
   out_6506538674473875324[6] = 0;
   out_6506538674473875324[7] = 0;
   out_6506538674473875324[8] = 0;
}
void h_26(double *state, double *unused, double *out_8978380754981091706) {
   out_8978380754981091706[0] = state[7];
}
void H_26(double *state, double *unused, double *out_7292731685727427298) {
   out_7292731685727427298[0] = 0;
   out_7292731685727427298[1] = 0;
   out_7292731685727427298[2] = 0;
   out_7292731685727427298[3] = 0;
   out_7292731685727427298[4] = 0;
   out_7292731685727427298[5] = 0;
   out_7292731685727427298[6] = 0;
   out_7292731685727427298[7] = 1;
   out_7292731685727427298[8] = 0;
}
void h_27(double *state, double *unused, double *out_6938698807011347957) {
   out_6938698807011347957[0] = state[3];
}
void H_27(double *state, double *unused, double *out_4331775362673450413) {
   out_4331775362673450413[0] = 0;
   out_4331775362673450413[1] = 0;
   out_4331775362673450413[2] = 0;
   out_4331775362673450413[3] = 1;
   out_4331775362673450413[4] = 0;
   out_4331775362673450413[5] = 0;
   out_4331775362673450413[6] = 0;
   out_4331775362673450413[7] = 0;
   out_4331775362673450413[8] = 0;
}
void h_29(double *state, double *unused, double *out_4741129432548446265) {
   out_4741129432548446265[0] = state[1];
}
void H_29(double *state, double *unused, double *out_7016770018788267508) {
   out_7016770018788267508[0] = 0;
   out_7016770018788267508[1] = 1;
   out_7016770018788267508[2] = 0;
   out_7016770018788267508[3] = 0;
   out_7016770018788267508[4] = 0;
   out_7016770018788267508[5] = 0;
   out_7016770018788267508[6] = 0;
   out_7016770018788267508[7] = 0;
   out_7016770018788267508[8] = 0;
}
void h_28(double *state, double *unused, double *out_64063896781717822) {
   out_64063896781717822[0] = state[0];
}
void H_28(double *state, double *unused, double *out_8980400290353593759) {
   out_8980400290353593759[0] = 1;
   out_8980400290353593759[1] = 0;
   out_8980400290353593759[2] = 0;
   out_8980400290353593759[3] = 0;
   out_8980400290353593759[4] = 0;
   out_8980400290353593759[5] = 0;
   out_8980400290353593759[6] = 0;
   out_8980400290353593759[7] = 0;
   out_8980400290353593759[8] = 0;
}
void h_31(double *state, double *unused, double *out_4885815735079099232) {
   out_4885815735079099232[0] = state[8];
}
void H_31(double *state, double *unused, double *out_6666523583494075822) {
   out_6666523583494075822[0] = 0;
   out_6666523583494075822[1] = 0;
   out_6666523583494075822[2] = 0;
   out_6666523583494075822[3] = 0;
   out_6666523583494075822[4] = 0;
   out_6666523583494075822[5] = 0;
   out_6666523583494075822[6] = 0;
   out_6666523583494075822[7] = 0;
   out_6666523583494075822[8] = 1;
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
void car_err_fun(double *nom_x, double *delta_x, double *out_417238876910560070) {
  err_fun(nom_x, delta_x, out_417238876910560070);
}
void car_inv_err_fun(double *nom_x, double *true_x, double *out_2214194102385980691) {
  inv_err_fun(nom_x, true_x, out_2214194102385980691);
}
void car_H_mod_fun(double *state, double *out_2617678226180198985) {
  H_mod_fun(state, out_2617678226180198985);
}
void car_f_fun(double *state, double dt, double *out_2248561019141826344) {
  f_fun(state,  dt, out_2248561019141826344);
}
void car_F_fun(double *state, double dt, double *out_1589062328883606575) {
  F_fun(state,  dt, out_1589062328883606575);
}
void car_h_25(double *state, double *unused, double *out_4251901549604751950) {
  h_25(state, unused, out_4251901549604751950);
}
void car_H_25(double *state, double *unused, double *out_7412509069108068094) {
  H_25(state, unused, out_7412509069108068094);
}
void car_h_24(double *state, double *unused, double *out_6482324594163419455) {
  h_24(state, unused, out_6482324594163419455);
}
void car_H_24(double *state, double *unused, double *out_1815556116961127131) {
  H_24(state, unused, out_1815556116961127131);
}
void car_h_30(double *state, double *unused, double *out_3976707487320246061) {
  h_30(state, unused, out_3976707487320246061);
}
void car_H_30(double *state, double *unused, double *out_6506538674473875324) {
  H_30(state, unused, out_6506538674473875324);
}
void car_h_26(double *state, double *unused, double *out_8978380754981091706) {
  h_26(state, unused, out_8978380754981091706);
}
void car_H_26(double *state, double *unused, double *out_7292731685727427298) {
  H_26(state, unused, out_7292731685727427298);
}
void car_h_27(double *state, double *unused, double *out_6938698807011347957) {
  h_27(state, unused, out_6938698807011347957);
}
void car_H_27(double *state, double *unused, double *out_4331775362673450413) {
  H_27(state, unused, out_4331775362673450413);
}
void car_h_29(double *state, double *unused, double *out_4741129432548446265) {
  h_29(state, unused, out_4741129432548446265);
}
void car_H_29(double *state, double *unused, double *out_7016770018788267508) {
  H_29(state, unused, out_7016770018788267508);
}
void car_h_28(double *state, double *unused, double *out_64063896781717822) {
  h_28(state, unused, out_64063896781717822);
}
void car_H_28(double *state, double *unused, double *out_8980400290353593759) {
  H_28(state, unused, out_8980400290353593759);
}
void car_h_31(double *state, double *unused, double *out_4885815735079099232) {
  h_31(state, unused, out_4885815735079099232);
}
void car_H_31(double *state, double *unused, double *out_6666523583494075822) {
  H_31(state, unused, out_6666523583494075822);
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
