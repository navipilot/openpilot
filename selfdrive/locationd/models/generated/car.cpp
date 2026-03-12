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
void err_fun(double *nom_x, double *delta_x, double *out_2689391300508487509) {
   out_2689391300508487509[0] = delta_x[0] + nom_x[0];
   out_2689391300508487509[1] = delta_x[1] + nom_x[1];
   out_2689391300508487509[2] = delta_x[2] + nom_x[2];
   out_2689391300508487509[3] = delta_x[3] + nom_x[3];
   out_2689391300508487509[4] = delta_x[4] + nom_x[4];
   out_2689391300508487509[5] = delta_x[5] + nom_x[5];
   out_2689391300508487509[6] = delta_x[6] + nom_x[6];
   out_2689391300508487509[7] = delta_x[7] + nom_x[7];
   out_2689391300508487509[8] = delta_x[8] + nom_x[8];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_7177783815062799236) {
   out_7177783815062799236[0] = -nom_x[0] + true_x[0];
   out_7177783815062799236[1] = -nom_x[1] + true_x[1];
   out_7177783815062799236[2] = -nom_x[2] + true_x[2];
   out_7177783815062799236[3] = -nom_x[3] + true_x[3];
   out_7177783815062799236[4] = -nom_x[4] + true_x[4];
   out_7177783815062799236[5] = -nom_x[5] + true_x[5];
   out_7177783815062799236[6] = -nom_x[6] + true_x[6];
   out_7177783815062799236[7] = -nom_x[7] + true_x[7];
   out_7177783815062799236[8] = -nom_x[8] + true_x[8];
}
void H_mod_fun(double *state, double *out_8221602802744378233) {
   out_8221602802744378233[0] = 1.0;
   out_8221602802744378233[1] = 0.0;
   out_8221602802744378233[2] = 0.0;
   out_8221602802744378233[3] = 0.0;
   out_8221602802744378233[4] = 0.0;
   out_8221602802744378233[5] = 0.0;
   out_8221602802744378233[6] = 0.0;
   out_8221602802744378233[7] = 0.0;
   out_8221602802744378233[8] = 0.0;
   out_8221602802744378233[9] = 0.0;
   out_8221602802744378233[10] = 1.0;
   out_8221602802744378233[11] = 0.0;
   out_8221602802744378233[12] = 0.0;
   out_8221602802744378233[13] = 0.0;
   out_8221602802744378233[14] = 0.0;
   out_8221602802744378233[15] = 0.0;
   out_8221602802744378233[16] = 0.0;
   out_8221602802744378233[17] = 0.0;
   out_8221602802744378233[18] = 0.0;
   out_8221602802744378233[19] = 0.0;
   out_8221602802744378233[20] = 1.0;
   out_8221602802744378233[21] = 0.0;
   out_8221602802744378233[22] = 0.0;
   out_8221602802744378233[23] = 0.0;
   out_8221602802744378233[24] = 0.0;
   out_8221602802744378233[25] = 0.0;
   out_8221602802744378233[26] = 0.0;
   out_8221602802744378233[27] = 0.0;
   out_8221602802744378233[28] = 0.0;
   out_8221602802744378233[29] = 0.0;
   out_8221602802744378233[30] = 1.0;
   out_8221602802744378233[31] = 0.0;
   out_8221602802744378233[32] = 0.0;
   out_8221602802744378233[33] = 0.0;
   out_8221602802744378233[34] = 0.0;
   out_8221602802744378233[35] = 0.0;
   out_8221602802744378233[36] = 0.0;
   out_8221602802744378233[37] = 0.0;
   out_8221602802744378233[38] = 0.0;
   out_8221602802744378233[39] = 0.0;
   out_8221602802744378233[40] = 1.0;
   out_8221602802744378233[41] = 0.0;
   out_8221602802744378233[42] = 0.0;
   out_8221602802744378233[43] = 0.0;
   out_8221602802744378233[44] = 0.0;
   out_8221602802744378233[45] = 0.0;
   out_8221602802744378233[46] = 0.0;
   out_8221602802744378233[47] = 0.0;
   out_8221602802744378233[48] = 0.0;
   out_8221602802744378233[49] = 0.0;
   out_8221602802744378233[50] = 1.0;
   out_8221602802744378233[51] = 0.0;
   out_8221602802744378233[52] = 0.0;
   out_8221602802744378233[53] = 0.0;
   out_8221602802744378233[54] = 0.0;
   out_8221602802744378233[55] = 0.0;
   out_8221602802744378233[56] = 0.0;
   out_8221602802744378233[57] = 0.0;
   out_8221602802744378233[58] = 0.0;
   out_8221602802744378233[59] = 0.0;
   out_8221602802744378233[60] = 1.0;
   out_8221602802744378233[61] = 0.0;
   out_8221602802744378233[62] = 0.0;
   out_8221602802744378233[63] = 0.0;
   out_8221602802744378233[64] = 0.0;
   out_8221602802744378233[65] = 0.0;
   out_8221602802744378233[66] = 0.0;
   out_8221602802744378233[67] = 0.0;
   out_8221602802744378233[68] = 0.0;
   out_8221602802744378233[69] = 0.0;
   out_8221602802744378233[70] = 1.0;
   out_8221602802744378233[71] = 0.0;
   out_8221602802744378233[72] = 0.0;
   out_8221602802744378233[73] = 0.0;
   out_8221602802744378233[74] = 0.0;
   out_8221602802744378233[75] = 0.0;
   out_8221602802744378233[76] = 0.0;
   out_8221602802744378233[77] = 0.0;
   out_8221602802744378233[78] = 0.0;
   out_8221602802744378233[79] = 0.0;
   out_8221602802744378233[80] = 1.0;
}
void f_fun(double *state, double dt, double *out_7651705913211861388) {
   out_7651705913211861388[0] = state[0];
   out_7651705913211861388[1] = state[1];
   out_7651705913211861388[2] = state[2];
   out_7651705913211861388[3] = state[3];
   out_7651705913211861388[4] = state[4];
   out_7651705913211861388[5] = dt*((-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]))*state[6] - 9.8100000000000005*state[8] + stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*state[1]) + (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*state[4])) + state[5];
   out_7651705913211861388[6] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*state[4])) + state[6];
   out_7651705913211861388[7] = state[7];
   out_7651705913211861388[8] = state[8];
}
void F_fun(double *state, double dt, double *out_97859790539154602) {
   out_97859790539154602[0] = 1;
   out_97859790539154602[1] = 0;
   out_97859790539154602[2] = 0;
   out_97859790539154602[3] = 0;
   out_97859790539154602[4] = 0;
   out_97859790539154602[5] = 0;
   out_97859790539154602[6] = 0;
   out_97859790539154602[7] = 0;
   out_97859790539154602[8] = 0;
   out_97859790539154602[9] = 0;
   out_97859790539154602[10] = 1;
   out_97859790539154602[11] = 0;
   out_97859790539154602[12] = 0;
   out_97859790539154602[13] = 0;
   out_97859790539154602[14] = 0;
   out_97859790539154602[15] = 0;
   out_97859790539154602[16] = 0;
   out_97859790539154602[17] = 0;
   out_97859790539154602[18] = 0;
   out_97859790539154602[19] = 0;
   out_97859790539154602[20] = 1;
   out_97859790539154602[21] = 0;
   out_97859790539154602[22] = 0;
   out_97859790539154602[23] = 0;
   out_97859790539154602[24] = 0;
   out_97859790539154602[25] = 0;
   out_97859790539154602[26] = 0;
   out_97859790539154602[27] = 0;
   out_97859790539154602[28] = 0;
   out_97859790539154602[29] = 0;
   out_97859790539154602[30] = 1;
   out_97859790539154602[31] = 0;
   out_97859790539154602[32] = 0;
   out_97859790539154602[33] = 0;
   out_97859790539154602[34] = 0;
   out_97859790539154602[35] = 0;
   out_97859790539154602[36] = 0;
   out_97859790539154602[37] = 0;
   out_97859790539154602[38] = 0;
   out_97859790539154602[39] = 0;
   out_97859790539154602[40] = 1;
   out_97859790539154602[41] = 0;
   out_97859790539154602[42] = 0;
   out_97859790539154602[43] = 0;
   out_97859790539154602[44] = 0;
   out_97859790539154602[45] = dt*(stiffness_front*(-state[2] - state[3] + state[7])/(mass*state[1]) + (-stiffness_front - stiffness_rear)*state[5]/(mass*state[4]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[6]/(mass*state[4]));
   out_97859790539154602[46] = -dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*pow(state[1], 2));
   out_97859790539154602[47] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_97859790539154602[48] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_97859790539154602[49] = dt*((-1 - (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*pow(state[4], 2)))*state[6] - (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*pow(state[4], 2)));
   out_97859790539154602[50] = dt*(-stiffness_front*state[0] - stiffness_rear*state[0])/(mass*state[4]) + 1;
   out_97859790539154602[51] = dt*(-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]));
   out_97859790539154602[52] = dt*stiffness_front*state[0]/(mass*state[1]);
   out_97859790539154602[53] = -9.8100000000000005*dt;
   out_97859790539154602[54] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front - pow(center_to_rear, 2)*stiffness_rear)*state[6]/(rotational_inertia*state[4]));
   out_97859790539154602[55] = -center_to_front*dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*pow(state[1], 2));
   out_97859790539154602[56] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_97859790539154602[57] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_97859790539154602[58] = dt*(-(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*pow(state[4], 2)) - (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*pow(state[4], 2)));
   out_97859790539154602[59] = dt*(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(rotational_inertia*state[4]);
   out_97859790539154602[60] = dt*(-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])/(rotational_inertia*state[4]) + 1;
   out_97859790539154602[61] = center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_97859790539154602[62] = 0;
   out_97859790539154602[63] = 0;
   out_97859790539154602[64] = 0;
   out_97859790539154602[65] = 0;
   out_97859790539154602[66] = 0;
   out_97859790539154602[67] = 0;
   out_97859790539154602[68] = 0;
   out_97859790539154602[69] = 0;
   out_97859790539154602[70] = 1;
   out_97859790539154602[71] = 0;
   out_97859790539154602[72] = 0;
   out_97859790539154602[73] = 0;
   out_97859790539154602[74] = 0;
   out_97859790539154602[75] = 0;
   out_97859790539154602[76] = 0;
   out_97859790539154602[77] = 0;
   out_97859790539154602[78] = 0;
   out_97859790539154602[79] = 0;
   out_97859790539154602[80] = 1;
}
void h_25(double *state, double *unused, double *out_1747255377487496462) {
   out_1747255377487496462[0] = state[6];
}
void H_25(double *state, double *unused, double *out_7588584345949747189) {
   out_7588584345949747189[0] = 0;
   out_7588584345949747189[1] = 0;
   out_7588584345949747189[2] = 0;
   out_7588584345949747189[3] = 0;
   out_7588584345949747189[4] = 0;
   out_7588584345949747189[5] = 0;
   out_7588584345949747189[6] = 1;
   out_7588584345949747189[7] = 0;
   out_7588584345949747189[8] = 0;
}
void h_24(double *state, double *unused, double *out_3070396560078381804) {
   out_3070396560078381804[0] = state[4];
   out_3070396560078381804[1] = state[5];
}
void H_24(double *state, double *unused, double *out_7300234128957481291) {
   out_7300234128957481291[0] = 0;
   out_7300234128957481291[1] = 0;
   out_7300234128957481291[2] = 0;
   out_7300234128957481291[3] = 0;
   out_7300234128957481291[4] = 1;
   out_7300234128957481291[5] = 0;
   out_7300234128957481291[6] = 0;
   out_7300234128957481291[7] = 0;
   out_7300234128957481291[8] = 0;
   out_7300234128957481291[9] = 0;
   out_7300234128957481291[10] = 0;
   out_7300234128957481291[11] = 0;
   out_7300234128957481291[12] = 0;
   out_7300234128957481291[13] = 0;
   out_7300234128957481291[14] = 1;
   out_7300234128957481291[15] = 0;
   out_7300234128957481291[16] = 0;
   out_7300234128957481291[17] = 0;
}
void h_30(double *state, double *unused, double *out_2594165583737162996) {
   out_2594165583737162996[0] = state[4];
}
void H_30(double *state, double *unused, double *out_8339826769252555800) {
   out_8339826769252555800[0] = 0;
   out_8339826769252555800[1] = 0;
   out_8339826769252555800[2] = 0;
   out_8339826769252555800[3] = 0;
   out_8339826769252555800[4] = 1;
   out_8339826769252555800[5] = 0;
   out_8339826769252555800[6] = 0;
   out_8339826769252555800[7] = 0;
   out_8339826769252555800[8] = 0;
}
void h_26(double *state, double *unused, double *out_1234538052727793168) {
   out_1234538052727793168[0] = state[7];
}
void H_26(double *state, double *unused, double *out_3847081027075690965) {
   out_3847081027075690965[0] = 0;
   out_3847081027075690965[1] = 0;
   out_3847081027075690965[2] = 0;
   out_3847081027075690965[3] = 0;
   out_3847081027075690965[4] = 0;
   out_3847081027075690965[5] = 0;
   out_3847081027075690965[6] = 0;
   out_3847081027075690965[7] = 1;
   out_3847081027075690965[8] = 0;
}
void h_27(double *state, double *unused, double *out_4784041428931829147) {
   out_4784041428931829147[0] = state[3];
}
void H_27(double *state, double *unused, double *out_886124704021714080) {
   out_886124704021714080[0] = 0;
   out_886124704021714080[1] = 0;
   out_886124704021714080[2] = 0;
   out_886124704021714080[3] = 1;
   out_886124704021714080[4] = 0;
   out_886124704021714080[5] = 0;
   out_886124704021714080[6] = 0;
   out_886124704021714080[7] = 0;
   out_886124704021714080[8] = 0;
}
void h_29(double *state, double *unused, double *out_2407507683923682649) {
   out_2407507683923682649[0] = state[1];
}
void H_29(double *state, double *unused, double *out_3571119360136531175) {
   out_3571119360136531175[0] = 0;
   out_3571119360136531175[1] = 1;
   out_3571119360136531175[2] = 0;
   out_3571119360136531175[3] = 0;
   out_3571119360136531175[4] = 0;
   out_3571119360136531175[5] = 0;
   out_3571119360136531175[6] = 0;
   out_3571119360136531175[7] = 0;
   out_3571119360136531175[8] = 0;
}
void h_28(double *state, double *unused, double *out_7679778805471580716) {
   out_7679778805471580716[0] = state[0];
}
void H_28(double *state, double *unused, double *out_5534749631701857426) {
   out_5534749631701857426[0] = 1;
   out_5534749631701857426[1] = 0;
   out_5534749631701857426[2] = 0;
   out_5534749631701857426[3] = 0;
   out_5534749631701857426[4] = 0;
   out_5534749631701857426[5] = 0;
   out_5534749631701857426[6] = 0;
   out_5534749631701857426[7] = 0;
   out_5534749631701857426[8] = 0;
}
void h_31(double *state, double *unused, double *out_5023579848862854474) {
   out_5023579848862854474[0] = state[8];
}
void H_31(double *state, double *unused, double *out_3220872924842339489) {
   out_3220872924842339489[0] = 0;
   out_3220872924842339489[1] = 0;
   out_3220872924842339489[2] = 0;
   out_3220872924842339489[3] = 0;
   out_3220872924842339489[4] = 0;
   out_3220872924842339489[5] = 0;
   out_3220872924842339489[6] = 0;
   out_3220872924842339489[7] = 0;
   out_3220872924842339489[8] = 1;
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
void car_err_fun(double *nom_x, double *delta_x, double *out_2689391300508487509) {
  err_fun(nom_x, delta_x, out_2689391300508487509);
}
void car_inv_err_fun(double *nom_x, double *true_x, double *out_7177783815062799236) {
  inv_err_fun(nom_x, true_x, out_7177783815062799236);
}
void car_H_mod_fun(double *state, double *out_8221602802744378233) {
  H_mod_fun(state, out_8221602802744378233);
}
void car_f_fun(double *state, double dt, double *out_7651705913211861388) {
  f_fun(state,  dt, out_7651705913211861388);
}
void car_F_fun(double *state, double dt, double *out_97859790539154602) {
  F_fun(state,  dt, out_97859790539154602);
}
void car_h_25(double *state, double *unused, double *out_1747255377487496462) {
  h_25(state, unused, out_1747255377487496462);
}
void car_H_25(double *state, double *unused, double *out_7588584345949747189) {
  H_25(state, unused, out_7588584345949747189);
}
void car_h_24(double *state, double *unused, double *out_3070396560078381804) {
  h_24(state, unused, out_3070396560078381804);
}
void car_H_24(double *state, double *unused, double *out_7300234128957481291) {
  H_24(state, unused, out_7300234128957481291);
}
void car_h_30(double *state, double *unused, double *out_2594165583737162996) {
  h_30(state, unused, out_2594165583737162996);
}
void car_H_30(double *state, double *unused, double *out_8339826769252555800) {
  H_30(state, unused, out_8339826769252555800);
}
void car_h_26(double *state, double *unused, double *out_1234538052727793168) {
  h_26(state, unused, out_1234538052727793168);
}
void car_H_26(double *state, double *unused, double *out_3847081027075690965) {
  H_26(state, unused, out_3847081027075690965);
}
void car_h_27(double *state, double *unused, double *out_4784041428931829147) {
  h_27(state, unused, out_4784041428931829147);
}
void car_H_27(double *state, double *unused, double *out_886124704021714080) {
  H_27(state, unused, out_886124704021714080);
}
void car_h_29(double *state, double *unused, double *out_2407507683923682649) {
  h_29(state, unused, out_2407507683923682649);
}
void car_H_29(double *state, double *unused, double *out_3571119360136531175) {
  H_29(state, unused, out_3571119360136531175);
}
void car_h_28(double *state, double *unused, double *out_7679778805471580716) {
  h_28(state, unused, out_7679778805471580716);
}
void car_H_28(double *state, double *unused, double *out_5534749631701857426) {
  H_28(state, unused, out_5534749631701857426);
}
void car_h_31(double *state, double *unused, double *out_5023579848862854474) {
  h_31(state, unused, out_5023579848862854474);
}
void car_H_31(double *state, double *unused, double *out_3220872924842339489) {
  H_31(state, unused, out_3220872924842339489);
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
