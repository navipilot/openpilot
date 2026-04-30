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
void err_fun(double *nom_x, double *delta_x, double *out_1814247348247547720) {
   out_1814247348247547720[0] = delta_x[0] + nom_x[0];
   out_1814247348247547720[1] = delta_x[1] + nom_x[1];
   out_1814247348247547720[2] = delta_x[2] + nom_x[2];
   out_1814247348247547720[3] = delta_x[3] + nom_x[3];
   out_1814247348247547720[4] = delta_x[4] + nom_x[4];
   out_1814247348247547720[5] = delta_x[5] + nom_x[5];
   out_1814247348247547720[6] = delta_x[6] + nom_x[6];
   out_1814247348247547720[7] = delta_x[7] + nom_x[7];
   out_1814247348247547720[8] = delta_x[8] + nom_x[8];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_2896416663387818009) {
   out_2896416663387818009[0] = -nom_x[0] + true_x[0];
   out_2896416663387818009[1] = -nom_x[1] + true_x[1];
   out_2896416663387818009[2] = -nom_x[2] + true_x[2];
   out_2896416663387818009[3] = -nom_x[3] + true_x[3];
   out_2896416663387818009[4] = -nom_x[4] + true_x[4];
   out_2896416663387818009[5] = -nom_x[5] + true_x[5];
   out_2896416663387818009[6] = -nom_x[6] + true_x[6];
   out_2896416663387818009[7] = -nom_x[7] + true_x[7];
   out_2896416663387818009[8] = -nom_x[8] + true_x[8];
}
void H_mod_fun(double *state, double *out_3833245775098430450) {
   out_3833245775098430450[0] = 1.0;
   out_3833245775098430450[1] = 0.0;
   out_3833245775098430450[2] = 0.0;
   out_3833245775098430450[3] = 0.0;
   out_3833245775098430450[4] = 0.0;
   out_3833245775098430450[5] = 0.0;
   out_3833245775098430450[6] = 0.0;
   out_3833245775098430450[7] = 0.0;
   out_3833245775098430450[8] = 0.0;
   out_3833245775098430450[9] = 0.0;
   out_3833245775098430450[10] = 1.0;
   out_3833245775098430450[11] = 0.0;
   out_3833245775098430450[12] = 0.0;
   out_3833245775098430450[13] = 0.0;
   out_3833245775098430450[14] = 0.0;
   out_3833245775098430450[15] = 0.0;
   out_3833245775098430450[16] = 0.0;
   out_3833245775098430450[17] = 0.0;
   out_3833245775098430450[18] = 0.0;
   out_3833245775098430450[19] = 0.0;
   out_3833245775098430450[20] = 1.0;
   out_3833245775098430450[21] = 0.0;
   out_3833245775098430450[22] = 0.0;
   out_3833245775098430450[23] = 0.0;
   out_3833245775098430450[24] = 0.0;
   out_3833245775098430450[25] = 0.0;
   out_3833245775098430450[26] = 0.0;
   out_3833245775098430450[27] = 0.0;
   out_3833245775098430450[28] = 0.0;
   out_3833245775098430450[29] = 0.0;
   out_3833245775098430450[30] = 1.0;
   out_3833245775098430450[31] = 0.0;
   out_3833245775098430450[32] = 0.0;
   out_3833245775098430450[33] = 0.0;
   out_3833245775098430450[34] = 0.0;
   out_3833245775098430450[35] = 0.0;
   out_3833245775098430450[36] = 0.0;
   out_3833245775098430450[37] = 0.0;
   out_3833245775098430450[38] = 0.0;
   out_3833245775098430450[39] = 0.0;
   out_3833245775098430450[40] = 1.0;
   out_3833245775098430450[41] = 0.0;
   out_3833245775098430450[42] = 0.0;
   out_3833245775098430450[43] = 0.0;
   out_3833245775098430450[44] = 0.0;
   out_3833245775098430450[45] = 0.0;
   out_3833245775098430450[46] = 0.0;
   out_3833245775098430450[47] = 0.0;
   out_3833245775098430450[48] = 0.0;
   out_3833245775098430450[49] = 0.0;
   out_3833245775098430450[50] = 1.0;
   out_3833245775098430450[51] = 0.0;
   out_3833245775098430450[52] = 0.0;
   out_3833245775098430450[53] = 0.0;
   out_3833245775098430450[54] = 0.0;
   out_3833245775098430450[55] = 0.0;
   out_3833245775098430450[56] = 0.0;
   out_3833245775098430450[57] = 0.0;
   out_3833245775098430450[58] = 0.0;
   out_3833245775098430450[59] = 0.0;
   out_3833245775098430450[60] = 1.0;
   out_3833245775098430450[61] = 0.0;
   out_3833245775098430450[62] = 0.0;
   out_3833245775098430450[63] = 0.0;
   out_3833245775098430450[64] = 0.0;
   out_3833245775098430450[65] = 0.0;
   out_3833245775098430450[66] = 0.0;
   out_3833245775098430450[67] = 0.0;
   out_3833245775098430450[68] = 0.0;
   out_3833245775098430450[69] = 0.0;
   out_3833245775098430450[70] = 1.0;
   out_3833245775098430450[71] = 0.0;
   out_3833245775098430450[72] = 0.0;
   out_3833245775098430450[73] = 0.0;
   out_3833245775098430450[74] = 0.0;
   out_3833245775098430450[75] = 0.0;
   out_3833245775098430450[76] = 0.0;
   out_3833245775098430450[77] = 0.0;
   out_3833245775098430450[78] = 0.0;
   out_3833245775098430450[79] = 0.0;
   out_3833245775098430450[80] = 1.0;
}
void f_fun(double *state, double dt, double *out_3477590770207651617) {
   out_3477590770207651617[0] = state[0];
   out_3477590770207651617[1] = state[1];
   out_3477590770207651617[2] = state[2];
   out_3477590770207651617[3] = state[3];
   out_3477590770207651617[4] = state[4];
   out_3477590770207651617[5] = dt*((-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]))*state[6] - 9.8100000000000005*state[8] + stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*state[1]) + (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*state[4])) + state[5];
   out_3477590770207651617[6] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*state[4])) + state[6];
   out_3477590770207651617[7] = state[7];
   out_3477590770207651617[8] = state[8];
}
void F_fun(double *state, double dt, double *out_3058811064297665734) {
   out_3058811064297665734[0] = 1;
   out_3058811064297665734[1] = 0;
   out_3058811064297665734[2] = 0;
   out_3058811064297665734[3] = 0;
   out_3058811064297665734[4] = 0;
   out_3058811064297665734[5] = 0;
   out_3058811064297665734[6] = 0;
   out_3058811064297665734[7] = 0;
   out_3058811064297665734[8] = 0;
   out_3058811064297665734[9] = 0;
   out_3058811064297665734[10] = 1;
   out_3058811064297665734[11] = 0;
   out_3058811064297665734[12] = 0;
   out_3058811064297665734[13] = 0;
   out_3058811064297665734[14] = 0;
   out_3058811064297665734[15] = 0;
   out_3058811064297665734[16] = 0;
   out_3058811064297665734[17] = 0;
   out_3058811064297665734[18] = 0;
   out_3058811064297665734[19] = 0;
   out_3058811064297665734[20] = 1;
   out_3058811064297665734[21] = 0;
   out_3058811064297665734[22] = 0;
   out_3058811064297665734[23] = 0;
   out_3058811064297665734[24] = 0;
   out_3058811064297665734[25] = 0;
   out_3058811064297665734[26] = 0;
   out_3058811064297665734[27] = 0;
   out_3058811064297665734[28] = 0;
   out_3058811064297665734[29] = 0;
   out_3058811064297665734[30] = 1;
   out_3058811064297665734[31] = 0;
   out_3058811064297665734[32] = 0;
   out_3058811064297665734[33] = 0;
   out_3058811064297665734[34] = 0;
   out_3058811064297665734[35] = 0;
   out_3058811064297665734[36] = 0;
   out_3058811064297665734[37] = 0;
   out_3058811064297665734[38] = 0;
   out_3058811064297665734[39] = 0;
   out_3058811064297665734[40] = 1;
   out_3058811064297665734[41] = 0;
   out_3058811064297665734[42] = 0;
   out_3058811064297665734[43] = 0;
   out_3058811064297665734[44] = 0;
   out_3058811064297665734[45] = dt*(stiffness_front*(-state[2] - state[3] + state[7])/(mass*state[1]) + (-stiffness_front - stiffness_rear)*state[5]/(mass*state[4]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[6]/(mass*state[4]));
   out_3058811064297665734[46] = -dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*pow(state[1], 2));
   out_3058811064297665734[47] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_3058811064297665734[48] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_3058811064297665734[49] = dt*((-1 - (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*pow(state[4], 2)))*state[6] - (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*pow(state[4], 2)));
   out_3058811064297665734[50] = dt*(-stiffness_front*state[0] - stiffness_rear*state[0])/(mass*state[4]) + 1;
   out_3058811064297665734[51] = dt*(-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]));
   out_3058811064297665734[52] = dt*stiffness_front*state[0]/(mass*state[1]);
   out_3058811064297665734[53] = -9.8100000000000005*dt;
   out_3058811064297665734[54] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front - pow(center_to_rear, 2)*stiffness_rear)*state[6]/(rotational_inertia*state[4]));
   out_3058811064297665734[55] = -center_to_front*dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*pow(state[1], 2));
   out_3058811064297665734[56] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_3058811064297665734[57] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_3058811064297665734[58] = dt*(-(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*pow(state[4], 2)) - (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*pow(state[4], 2)));
   out_3058811064297665734[59] = dt*(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(rotational_inertia*state[4]);
   out_3058811064297665734[60] = dt*(-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])/(rotational_inertia*state[4]) + 1;
   out_3058811064297665734[61] = center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_3058811064297665734[62] = 0;
   out_3058811064297665734[63] = 0;
   out_3058811064297665734[64] = 0;
   out_3058811064297665734[65] = 0;
   out_3058811064297665734[66] = 0;
   out_3058811064297665734[67] = 0;
   out_3058811064297665734[68] = 0;
   out_3058811064297665734[69] = 0;
   out_3058811064297665734[70] = 1;
   out_3058811064297665734[71] = 0;
   out_3058811064297665734[72] = 0;
   out_3058811064297665734[73] = 0;
   out_3058811064297665734[74] = 0;
   out_3058811064297665734[75] = 0;
   out_3058811064297665734[76] = 0;
   out_3058811064297665734[77] = 0;
   out_3058811064297665734[78] = 0;
   out_3058811064297665734[79] = 0;
   out_3058811064297665734[80] = 1;
}
void h_25(double *state, double *unused, double *out_845892813044312576) {
   out_845892813044312576[0] = state[6];
}
void H_25(double *state, double *unused, double *out_6744590887944551998) {
   out_6744590887944551998[0] = 0;
   out_6744590887944551998[1] = 0;
   out_6744590887944551998[2] = 0;
   out_6744590887944551998[3] = 0;
   out_6744590887944551998[4] = 0;
   out_6744590887944551998[5] = 0;
   out_6744590887944551998[6] = 1;
   out_6744590887944551998[7] = 0;
   out_6744590887944551998[8] = 0;
}
void h_24(double *state, double *unused, double *out_410909672515548225) {
   out_410909672515548225[0] = state[4];
   out_410909672515548225[1] = state[5];
}
void H_24(double *state, double *unused, double *out_673800745732374895) {
   out_673800745732374895[0] = 0;
   out_673800745732374895[1] = 0;
   out_673800745732374895[2] = 0;
   out_673800745732374895[3] = 0;
   out_673800745732374895[4] = 1;
   out_673800745732374895[5] = 0;
   out_673800745732374895[6] = 0;
   out_673800745732374895[7] = 0;
   out_673800745732374895[8] = 0;
   out_673800745732374895[9] = 0;
   out_673800745732374895[10] = 0;
   out_673800745732374895[11] = 0;
   out_673800745732374895[12] = 0;
   out_673800745732374895[13] = 0;
   out_673800745732374895[14] = 1;
   out_673800745732374895[15] = 0;
   out_673800745732374895[16] = 0;
   out_673800745732374895[17] = 0;
}
void h_30(double *state, double *unused, double *out_8727165463798361761) {
   out_8727165463798361761[0] = state[4];
}
void H_30(double *state, double *unused, double *out_7174456855637391420) {
   out_7174456855637391420[0] = 0;
   out_7174456855637391420[1] = 0;
   out_7174456855637391420[2] = 0;
   out_7174456855637391420[3] = 0;
   out_7174456855637391420[4] = 1;
   out_7174456855637391420[5] = 0;
   out_7174456855637391420[6] = 0;
   out_7174456855637391420[7] = 0;
   out_7174456855637391420[8] = 0;
}
void h_26(double *state, double *unused, double *out_2811534710064499425) {
   out_2811534710064499425[0] = state[7];
}
void H_26(double *state, double *unused, double *out_7960649866890943394) {
   out_7960649866890943394[0] = 0;
   out_7960649866890943394[1] = 0;
   out_7960649866890943394[2] = 0;
   out_7960649866890943394[3] = 0;
   out_7960649866890943394[4] = 0;
   out_7960649866890943394[5] = 0;
   out_7960649866890943394[6] = 0;
   out_7960649866890943394[7] = 1;
   out_7960649866890943394[8] = 0;
}
void h_27(double *state, double *unused, double *out_4116487184636980656) {
   out_4116487184636980656[0] = state[3];
}
void H_27(double *state, double *unused, double *out_9048693146888216979) {
   out_9048693146888216979[0] = 0;
   out_9048693146888216979[1] = 0;
   out_9048693146888216979[2] = 0;
   out_9048693146888216979[3] = 1;
   out_9048693146888216979[4] = 0;
   out_9048693146888216979[5] = 0;
   out_9048693146888216979[6] = 0;
   out_9048693146888216979[7] = 0;
   out_9048693146888216979[8] = 0;
}
void h_29(double *state, double *unused, double *out_1915699176115514220) {
   out_1915699176115514220[0] = state[1];
}
void H_29(double *state, double *unused, double *out_7684688199951783604) {
   out_7684688199951783604[0] = 0;
   out_7684688199951783604[1] = 1;
   out_7684688199951783604[2] = 0;
   out_7684688199951783604[3] = 0;
   out_7684688199951783604[4] = 0;
   out_7684688199951783604[5] = 0;
   out_7684688199951783604[6] = 0;
   out_7684688199951783604[7] = 0;
   out_7684688199951783604[8] = 0;
}
void h_28(double *state, double *unused, double *out_6496658557271199005) {
   out_6496658557271199005[0] = state[0];
}
void H_28(double *state, double *unused, double *out_2602289182882253030) {
   out_2602289182882253030[0] = 1;
   out_2602289182882253030[1] = 0;
   out_2602289182882253030[2] = 0;
   out_2602289182882253030[3] = 0;
   out_2602289182882253030[4] = 0;
   out_2602289182882253030[5] = 0;
   out_2602289182882253030[6] = 0;
   out_2602289182882253030[7] = 0;
   out_2602289182882253030[8] = 0;
}
void h_31(double *state, double *unused, double *out_9104190579401499048) {
   out_9104190579401499048[0] = state[8];
}
void H_31(double *state, double *unused, double *out_7334441764657591918) {
   out_7334441764657591918[0] = 0;
   out_7334441764657591918[1] = 0;
   out_7334441764657591918[2] = 0;
   out_7334441764657591918[3] = 0;
   out_7334441764657591918[4] = 0;
   out_7334441764657591918[5] = 0;
   out_7334441764657591918[6] = 0;
   out_7334441764657591918[7] = 0;
   out_7334441764657591918[8] = 1;
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
void car_err_fun(double *nom_x, double *delta_x, double *out_1814247348247547720) {
  err_fun(nom_x, delta_x, out_1814247348247547720);
}
void car_inv_err_fun(double *nom_x, double *true_x, double *out_2896416663387818009) {
  inv_err_fun(nom_x, true_x, out_2896416663387818009);
}
void car_H_mod_fun(double *state, double *out_3833245775098430450) {
  H_mod_fun(state, out_3833245775098430450);
}
void car_f_fun(double *state, double dt, double *out_3477590770207651617) {
  f_fun(state,  dt, out_3477590770207651617);
}
void car_F_fun(double *state, double dt, double *out_3058811064297665734) {
  F_fun(state,  dt, out_3058811064297665734);
}
void car_h_25(double *state, double *unused, double *out_845892813044312576) {
  h_25(state, unused, out_845892813044312576);
}
void car_H_25(double *state, double *unused, double *out_6744590887944551998) {
  H_25(state, unused, out_6744590887944551998);
}
void car_h_24(double *state, double *unused, double *out_410909672515548225) {
  h_24(state, unused, out_410909672515548225);
}
void car_H_24(double *state, double *unused, double *out_673800745732374895) {
  H_24(state, unused, out_673800745732374895);
}
void car_h_30(double *state, double *unused, double *out_8727165463798361761) {
  h_30(state, unused, out_8727165463798361761);
}
void car_H_30(double *state, double *unused, double *out_7174456855637391420) {
  H_30(state, unused, out_7174456855637391420);
}
void car_h_26(double *state, double *unused, double *out_2811534710064499425) {
  h_26(state, unused, out_2811534710064499425);
}
void car_H_26(double *state, double *unused, double *out_7960649866890943394) {
  H_26(state, unused, out_7960649866890943394);
}
void car_h_27(double *state, double *unused, double *out_4116487184636980656) {
  h_27(state, unused, out_4116487184636980656);
}
void car_H_27(double *state, double *unused, double *out_9048693146888216979) {
  H_27(state, unused, out_9048693146888216979);
}
void car_h_29(double *state, double *unused, double *out_1915699176115514220) {
  h_29(state, unused, out_1915699176115514220);
}
void car_H_29(double *state, double *unused, double *out_7684688199951783604) {
  H_29(state, unused, out_7684688199951783604);
}
void car_h_28(double *state, double *unused, double *out_6496658557271199005) {
  h_28(state, unused, out_6496658557271199005);
}
void car_H_28(double *state, double *unused, double *out_2602289182882253030) {
  H_28(state, unused, out_2602289182882253030);
}
void car_h_31(double *state, double *unused, double *out_9104190579401499048) {
  h_31(state, unused, out_9104190579401499048);
}
void car_H_31(double *state, double *unused, double *out_7334441764657591918) {
  H_31(state, unused, out_7334441764657591918);
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
