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
void err_fun(double *nom_x, double *delta_x, double *out_2687587277675065552) {
   out_2687587277675065552[0] = delta_x[0] + nom_x[0];
   out_2687587277675065552[1] = delta_x[1] + nom_x[1];
   out_2687587277675065552[2] = delta_x[2] + nom_x[2];
   out_2687587277675065552[3] = delta_x[3] + nom_x[3];
   out_2687587277675065552[4] = delta_x[4] + nom_x[4];
   out_2687587277675065552[5] = delta_x[5] + nom_x[5];
   out_2687587277675065552[6] = delta_x[6] + nom_x[6];
   out_2687587277675065552[7] = delta_x[7] + nom_x[7];
   out_2687587277675065552[8] = delta_x[8] + nom_x[8];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_6222700140799525089) {
   out_6222700140799525089[0] = -nom_x[0] + true_x[0];
   out_6222700140799525089[1] = -nom_x[1] + true_x[1];
   out_6222700140799525089[2] = -nom_x[2] + true_x[2];
   out_6222700140799525089[3] = -nom_x[3] + true_x[3];
   out_6222700140799525089[4] = -nom_x[4] + true_x[4];
   out_6222700140799525089[5] = -nom_x[5] + true_x[5];
   out_6222700140799525089[6] = -nom_x[6] + true_x[6];
   out_6222700140799525089[7] = -nom_x[7] + true_x[7];
   out_6222700140799525089[8] = -nom_x[8] + true_x[8];
}
void H_mod_fun(double *state, double *out_5736311190330935470) {
   out_5736311190330935470[0] = 1.0;
   out_5736311190330935470[1] = 0.0;
   out_5736311190330935470[2] = 0.0;
   out_5736311190330935470[3] = 0.0;
   out_5736311190330935470[4] = 0.0;
   out_5736311190330935470[5] = 0.0;
   out_5736311190330935470[6] = 0.0;
   out_5736311190330935470[7] = 0.0;
   out_5736311190330935470[8] = 0.0;
   out_5736311190330935470[9] = 0.0;
   out_5736311190330935470[10] = 1.0;
   out_5736311190330935470[11] = 0.0;
   out_5736311190330935470[12] = 0.0;
   out_5736311190330935470[13] = 0.0;
   out_5736311190330935470[14] = 0.0;
   out_5736311190330935470[15] = 0.0;
   out_5736311190330935470[16] = 0.0;
   out_5736311190330935470[17] = 0.0;
   out_5736311190330935470[18] = 0.0;
   out_5736311190330935470[19] = 0.0;
   out_5736311190330935470[20] = 1.0;
   out_5736311190330935470[21] = 0.0;
   out_5736311190330935470[22] = 0.0;
   out_5736311190330935470[23] = 0.0;
   out_5736311190330935470[24] = 0.0;
   out_5736311190330935470[25] = 0.0;
   out_5736311190330935470[26] = 0.0;
   out_5736311190330935470[27] = 0.0;
   out_5736311190330935470[28] = 0.0;
   out_5736311190330935470[29] = 0.0;
   out_5736311190330935470[30] = 1.0;
   out_5736311190330935470[31] = 0.0;
   out_5736311190330935470[32] = 0.0;
   out_5736311190330935470[33] = 0.0;
   out_5736311190330935470[34] = 0.0;
   out_5736311190330935470[35] = 0.0;
   out_5736311190330935470[36] = 0.0;
   out_5736311190330935470[37] = 0.0;
   out_5736311190330935470[38] = 0.0;
   out_5736311190330935470[39] = 0.0;
   out_5736311190330935470[40] = 1.0;
   out_5736311190330935470[41] = 0.0;
   out_5736311190330935470[42] = 0.0;
   out_5736311190330935470[43] = 0.0;
   out_5736311190330935470[44] = 0.0;
   out_5736311190330935470[45] = 0.0;
   out_5736311190330935470[46] = 0.0;
   out_5736311190330935470[47] = 0.0;
   out_5736311190330935470[48] = 0.0;
   out_5736311190330935470[49] = 0.0;
   out_5736311190330935470[50] = 1.0;
   out_5736311190330935470[51] = 0.0;
   out_5736311190330935470[52] = 0.0;
   out_5736311190330935470[53] = 0.0;
   out_5736311190330935470[54] = 0.0;
   out_5736311190330935470[55] = 0.0;
   out_5736311190330935470[56] = 0.0;
   out_5736311190330935470[57] = 0.0;
   out_5736311190330935470[58] = 0.0;
   out_5736311190330935470[59] = 0.0;
   out_5736311190330935470[60] = 1.0;
   out_5736311190330935470[61] = 0.0;
   out_5736311190330935470[62] = 0.0;
   out_5736311190330935470[63] = 0.0;
   out_5736311190330935470[64] = 0.0;
   out_5736311190330935470[65] = 0.0;
   out_5736311190330935470[66] = 0.0;
   out_5736311190330935470[67] = 0.0;
   out_5736311190330935470[68] = 0.0;
   out_5736311190330935470[69] = 0.0;
   out_5736311190330935470[70] = 1.0;
   out_5736311190330935470[71] = 0.0;
   out_5736311190330935470[72] = 0.0;
   out_5736311190330935470[73] = 0.0;
   out_5736311190330935470[74] = 0.0;
   out_5736311190330935470[75] = 0.0;
   out_5736311190330935470[76] = 0.0;
   out_5736311190330935470[77] = 0.0;
   out_5736311190330935470[78] = 0.0;
   out_5736311190330935470[79] = 0.0;
   out_5736311190330935470[80] = 1.0;
}
void f_fun(double *state, double dt, double *out_8059955085909907268) {
   out_8059955085909907268[0] = state[0];
   out_8059955085909907268[1] = state[1];
   out_8059955085909907268[2] = state[2];
   out_8059955085909907268[3] = state[3];
   out_8059955085909907268[4] = state[4];
   out_8059955085909907268[5] = dt*((-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]))*state[6] - 9.8100000000000005*state[8] + stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*state[1]) + (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*state[4])) + state[5];
   out_8059955085909907268[6] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*state[4])) + state[6];
   out_8059955085909907268[7] = state[7];
   out_8059955085909907268[8] = state[8];
}
void F_fun(double *state, double dt, double *out_535217138807217635) {
   out_535217138807217635[0] = 1;
   out_535217138807217635[1] = 0;
   out_535217138807217635[2] = 0;
   out_535217138807217635[3] = 0;
   out_535217138807217635[4] = 0;
   out_535217138807217635[5] = 0;
   out_535217138807217635[6] = 0;
   out_535217138807217635[7] = 0;
   out_535217138807217635[8] = 0;
   out_535217138807217635[9] = 0;
   out_535217138807217635[10] = 1;
   out_535217138807217635[11] = 0;
   out_535217138807217635[12] = 0;
   out_535217138807217635[13] = 0;
   out_535217138807217635[14] = 0;
   out_535217138807217635[15] = 0;
   out_535217138807217635[16] = 0;
   out_535217138807217635[17] = 0;
   out_535217138807217635[18] = 0;
   out_535217138807217635[19] = 0;
   out_535217138807217635[20] = 1;
   out_535217138807217635[21] = 0;
   out_535217138807217635[22] = 0;
   out_535217138807217635[23] = 0;
   out_535217138807217635[24] = 0;
   out_535217138807217635[25] = 0;
   out_535217138807217635[26] = 0;
   out_535217138807217635[27] = 0;
   out_535217138807217635[28] = 0;
   out_535217138807217635[29] = 0;
   out_535217138807217635[30] = 1;
   out_535217138807217635[31] = 0;
   out_535217138807217635[32] = 0;
   out_535217138807217635[33] = 0;
   out_535217138807217635[34] = 0;
   out_535217138807217635[35] = 0;
   out_535217138807217635[36] = 0;
   out_535217138807217635[37] = 0;
   out_535217138807217635[38] = 0;
   out_535217138807217635[39] = 0;
   out_535217138807217635[40] = 1;
   out_535217138807217635[41] = 0;
   out_535217138807217635[42] = 0;
   out_535217138807217635[43] = 0;
   out_535217138807217635[44] = 0;
   out_535217138807217635[45] = dt*(stiffness_front*(-state[2] - state[3] + state[7])/(mass*state[1]) + (-stiffness_front - stiffness_rear)*state[5]/(mass*state[4]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[6]/(mass*state[4]));
   out_535217138807217635[46] = -dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*pow(state[1], 2));
   out_535217138807217635[47] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_535217138807217635[48] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_535217138807217635[49] = dt*((-1 - (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*pow(state[4], 2)))*state[6] - (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*pow(state[4], 2)));
   out_535217138807217635[50] = dt*(-stiffness_front*state[0] - stiffness_rear*state[0])/(mass*state[4]) + 1;
   out_535217138807217635[51] = dt*(-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]));
   out_535217138807217635[52] = dt*stiffness_front*state[0]/(mass*state[1]);
   out_535217138807217635[53] = -9.8100000000000005*dt;
   out_535217138807217635[54] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front - pow(center_to_rear, 2)*stiffness_rear)*state[6]/(rotational_inertia*state[4]));
   out_535217138807217635[55] = -center_to_front*dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*pow(state[1], 2));
   out_535217138807217635[56] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_535217138807217635[57] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_535217138807217635[58] = dt*(-(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*pow(state[4], 2)) - (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*pow(state[4], 2)));
   out_535217138807217635[59] = dt*(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(rotational_inertia*state[4]);
   out_535217138807217635[60] = dt*(-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])/(rotational_inertia*state[4]) + 1;
   out_535217138807217635[61] = center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_535217138807217635[62] = 0;
   out_535217138807217635[63] = 0;
   out_535217138807217635[64] = 0;
   out_535217138807217635[65] = 0;
   out_535217138807217635[66] = 0;
   out_535217138807217635[67] = 0;
   out_535217138807217635[68] = 0;
   out_535217138807217635[69] = 0;
   out_535217138807217635[70] = 1;
   out_535217138807217635[71] = 0;
   out_535217138807217635[72] = 0;
   out_535217138807217635[73] = 0;
   out_535217138807217635[74] = 0;
   out_535217138807217635[75] = 0;
   out_535217138807217635[76] = 0;
   out_535217138807217635[77] = 0;
   out_535217138807217635[78] = 0;
   out_535217138807217635[79] = 0;
   out_535217138807217635[80] = 1;
}
void h_25(double *state, double *unused, double *out_7527307434081215851) {
   out_7527307434081215851[0] = state[6];
}
void H_25(double *state, double *unused, double *out_9187452233389124469) {
   out_9187452233389124469[0] = 0;
   out_9187452233389124469[1] = 0;
   out_9187452233389124469[2] = 0;
   out_9187452233389124469[3] = 0;
   out_9187452233389124469[4] = 0;
   out_9187452233389124469[5] = 0;
   out_9187452233389124469[6] = 1;
   out_9187452233389124469[7] = 0;
   out_9187452233389124469[8] = 0;
}
void h_24(double *state, double *unused, double *out_7476062049775574580) {
   out_7476062049775574580[0] = state[4];
   out_7476062049775574580[1] = state[5];
}
void H_24(double *state, double *unused, double *out_5629526634586801333) {
   out_5629526634586801333[0] = 0;
   out_5629526634586801333[1] = 0;
   out_5629526634586801333[2] = 0;
   out_5629526634586801333[3] = 0;
   out_5629526634586801333[4] = 1;
   out_5629526634586801333[5] = 0;
   out_5629526634586801333[6] = 0;
   out_5629526634586801333[7] = 0;
   out_5629526634586801333[8] = 0;
   out_5629526634586801333[9] = 0;
   out_5629526634586801333[10] = 0;
   out_5629526634586801333[11] = 0;
   out_5629526634586801333[12] = 0;
   out_5629526634586801333[13] = 0;
   out_5629526634586801333[14] = 1;
   out_5629526634586801333[15] = 0;
   out_5629526634586801333[16] = 0;
   out_5629526634586801333[17] = 0;
}
void h_30(double *state, double *unused, double *out_1371860238240555722) {
   out_1371860238240555722[0] = state[4];
}
void H_30(double *state, double *unused, double *out_2270761891897507714) {
   out_2270761891897507714[0] = 0;
   out_2270761891897507714[1] = 0;
   out_2270761891897507714[2] = 0;
   out_2270761891897507714[3] = 0;
   out_2270761891897507714[4] = 1;
   out_2270761891897507714[5] = 0;
   out_2270761891897507714[6] = 0;
   out_2270761891897507714[7] = 0;
   out_2270761891897507714[8] = 0;
}
void h_26(double *state, double *unused, double *out_6063283196188145091) {
   out_6063283196188145091[0] = state[7];
}
void H_26(double *state, double *unused, double *out_5517788521446370923) {
   out_5517788521446370923[0] = 0;
   out_5517788521446370923[1] = 0;
   out_5517788521446370923[2] = 0;
   out_5517788521446370923[3] = 0;
   out_5517788521446370923[4] = 0;
   out_5517788521446370923[5] = 0;
   out_5517788521446370923[6] = 0;
   out_5517788521446370923[7] = 1;
   out_5517788521446370923[8] = 0;
}
void h_27(double *state, double *unused, double *out_8470817286982878286) {
   out_8470817286982878286[0] = state[3];
}
void H_27(double *state, double *unused, double *out_4445525203697932625) {
   out_4445525203697932625[0] = 0;
   out_4445525203697932625[1] = 0;
   out_4445525203697932625[2] = 0;
   out_4445525203697932625[3] = 1;
   out_4445525203697932625[4] = 0;
   out_4445525203697932625[5] = 0;
   out_4445525203697932625[6] = 0;
   out_4445525203697932625[7] = 0;
   out_4445525203697932625[8] = 0;
}
void h_29(double *state, double *unused, double *out_7424449226025441418) {
   out_7424449226025441418[0] = state[1];
}
void H_29(double *state, double *unused, double *out_1760530547583115530) {
   out_1760530547583115530[0] = 0;
   out_1760530547583115530[1] = 1;
   out_1760530547583115530[2] = 0;
   out_1760530547583115530[3] = 0;
   out_1760530547583115530[4] = 0;
   out_1760530547583115530[5] = 0;
   out_1760530547583115530[6] = 0;
   out_1760530547583115530[7] = 0;
   out_1760530547583115530[8] = 0;
}
void h_28(double *state, double *unused, double *out_2540783352297972517) {
   out_2540783352297972517[0] = state[0];
}
void H_28(double *state, double *unused, double *out_6842929564652646104) {
   out_6842929564652646104[0] = 1;
   out_6842929564652646104[1] = 0;
   out_6842929564652646104[2] = 0;
   out_6842929564652646104[3] = 0;
   out_6842929564652646104[4] = 0;
   out_6842929564652646104[5] = 0;
   out_6842929564652646104[6] = 0;
   out_6842929564652646104[7] = 0;
   out_6842929564652646104[8] = 0;
}
void h_31(double *state, double *unused, double *out_7802501496365721740) {
   out_7802501496365721740[0] = state[8];
}
void H_31(double *state, double *unused, double *out_9156806271512164041) {
   out_9156806271512164041[0] = 0;
   out_9156806271512164041[1] = 0;
   out_9156806271512164041[2] = 0;
   out_9156806271512164041[3] = 0;
   out_9156806271512164041[4] = 0;
   out_9156806271512164041[5] = 0;
   out_9156806271512164041[6] = 0;
   out_9156806271512164041[7] = 0;
   out_9156806271512164041[8] = 1;
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
void car_err_fun(double *nom_x, double *delta_x, double *out_2687587277675065552) {
  err_fun(nom_x, delta_x, out_2687587277675065552);
}
void car_inv_err_fun(double *nom_x, double *true_x, double *out_6222700140799525089) {
  inv_err_fun(nom_x, true_x, out_6222700140799525089);
}
void car_H_mod_fun(double *state, double *out_5736311190330935470) {
  H_mod_fun(state, out_5736311190330935470);
}
void car_f_fun(double *state, double dt, double *out_8059955085909907268) {
  f_fun(state,  dt, out_8059955085909907268);
}
void car_F_fun(double *state, double dt, double *out_535217138807217635) {
  F_fun(state,  dt, out_535217138807217635);
}
void car_h_25(double *state, double *unused, double *out_7527307434081215851) {
  h_25(state, unused, out_7527307434081215851);
}
void car_H_25(double *state, double *unused, double *out_9187452233389124469) {
  H_25(state, unused, out_9187452233389124469);
}
void car_h_24(double *state, double *unused, double *out_7476062049775574580) {
  h_24(state, unused, out_7476062049775574580);
}
void car_H_24(double *state, double *unused, double *out_5629526634586801333) {
  H_24(state, unused, out_5629526634586801333);
}
void car_h_30(double *state, double *unused, double *out_1371860238240555722) {
  h_30(state, unused, out_1371860238240555722);
}
void car_H_30(double *state, double *unused, double *out_2270761891897507714) {
  H_30(state, unused, out_2270761891897507714);
}
void car_h_26(double *state, double *unused, double *out_6063283196188145091) {
  h_26(state, unused, out_6063283196188145091);
}
void car_H_26(double *state, double *unused, double *out_5517788521446370923) {
  H_26(state, unused, out_5517788521446370923);
}
void car_h_27(double *state, double *unused, double *out_8470817286982878286) {
  h_27(state, unused, out_8470817286982878286);
}
void car_H_27(double *state, double *unused, double *out_4445525203697932625) {
  H_27(state, unused, out_4445525203697932625);
}
void car_h_29(double *state, double *unused, double *out_7424449226025441418) {
  h_29(state, unused, out_7424449226025441418);
}
void car_H_29(double *state, double *unused, double *out_1760530547583115530) {
  H_29(state, unused, out_1760530547583115530);
}
void car_h_28(double *state, double *unused, double *out_2540783352297972517) {
  h_28(state, unused, out_2540783352297972517);
}
void car_H_28(double *state, double *unused, double *out_6842929564652646104) {
  H_28(state, unused, out_6842929564652646104);
}
void car_h_31(double *state, double *unused, double *out_7802501496365721740) {
  h_31(state, unused, out_7802501496365721740);
}
void car_H_31(double *state, double *unused, double *out_9156806271512164041) {
  H_31(state, unused, out_9156806271512164041);
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
