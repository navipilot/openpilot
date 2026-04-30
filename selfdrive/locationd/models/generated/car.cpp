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
void err_fun(double *nom_x, double *delta_x, double *out_8235846690843231890) {
   out_8235846690843231890[0] = delta_x[0] + nom_x[0];
   out_8235846690843231890[1] = delta_x[1] + nom_x[1];
   out_8235846690843231890[2] = delta_x[2] + nom_x[2];
   out_8235846690843231890[3] = delta_x[3] + nom_x[3];
   out_8235846690843231890[4] = delta_x[4] + nom_x[4];
   out_8235846690843231890[5] = delta_x[5] + nom_x[5];
   out_8235846690843231890[6] = delta_x[6] + nom_x[6];
   out_8235846690843231890[7] = delta_x[7] + nom_x[7];
   out_8235846690843231890[8] = delta_x[8] + nom_x[8];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_4073537797462342510) {
   out_4073537797462342510[0] = -nom_x[0] + true_x[0];
   out_4073537797462342510[1] = -nom_x[1] + true_x[1];
   out_4073537797462342510[2] = -nom_x[2] + true_x[2];
   out_4073537797462342510[3] = -nom_x[3] + true_x[3];
   out_4073537797462342510[4] = -nom_x[4] + true_x[4];
   out_4073537797462342510[5] = -nom_x[5] + true_x[5];
   out_4073537797462342510[6] = -nom_x[6] + true_x[6];
   out_4073537797462342510[7] = -nom_x[7] + true_x[7];
   out_4073537797462342510[8] = -nom_x[8] + true_x[8];
}
void H_mod_fun(double *state, double *out_1939718218409672785) {
   out_1939718218409672785[0] = 1.0;
   out_1939718218409672785[1] = 0.0;
   out_1939718218409672785[2] = 0.0;
   out_1939718218409672785[3] = 0.0;
   out_1939718218409672785[4] = 0.0;
   out_1939718218409672785[5] = 0.0;
   out_1939718218409672785[6] = 0.0;
   out_1939718218409672785[7] = 0.0;
   out_1939718218409672785[8] = 0.0;
   out_1939718218409672785[9] = 0.0;
   out_1939718218409672785[10] = 1.0;
   out_1939718218409672785[11] = 0.0;
   out_1939718218409672785[12] = 0.0;
   out_1939718218409672785[13] = 0.0;
   out_1939718218409672785[14] = 0.0;
   out_1939718218409672785[15] = 0.0;
   out_1939718218409672785[16] = 0.0;
   out_1939718218409672785[17] = 0.0;
   out_1939718218409672785[18] = 0.0;
   out_1939718218409672785[19] = 0.0;
   out_1939718218409672785[20] = 1.0;
   out_1939718218409672785[21] = 0.0;
   out_1939718218409672785[22] = 0.0;
   out_1939718218409672785[23] = 0.0;
   out_1939718218409672785[24] = 0.0;
   out_1939718218409672785[25] = 0.0;
   out_1939718218409672785[26] = 0.0;
   out_1939718218409672785[27] = 0.0;
   out_1939718218409672785[28] = 0.0;
   out_1939718218409672785[29] = 0.0;
   out_1939718218409672785[30] = 1.0;
   out_1939718218409672785[31] = 0.0;
   out_1939718218409672785[32] = 0.0;
   out_1939718218409672785[33] = 0.0;
   out_1939718218409672785[34] = 0.0;
   out_1939718218409672785[35] = 0.0;
   out_1939718218409672785[36] = 0.0;
   out_1939718218409672785[37] = 0.0;
   out_1939718218409672785[38] = 0.0;
   out_1939718218409672785[39] = 0.0;
   out_1939718218409672785[40] = 1.0;
   out_1939718218409672785[41] = 0.0;
   out_1939718218409672785[42] = 0.0;
   out_1939718218409672785[43] = 0.0;
   out_1939718218409672785[44] = 0.0;
   out_1939718218409672785[45] = 0.0;
   out_1939718218409672785[46] = 0.0;
   out_1939718218409672785[47] = 0.0;
   out_1939718218409672785[48] = 0.0;
   out_1939718218409672785[49] = 0.0;
   out_1939718218409672785[50] = 1.0;
   out_1939718218409672785[51] = 0.0;
   out_1939718218409672785[52] = 0.0;
   out_1939718218409672785[53] = 0.0;
   out_1939718218409672785[54] = 0.0;
   out_1939718218409672785[55] = 0.0;
   out_1939718218409672785[56] = 0.0;
   out_1939718218409672785[57] = 0.0;
   out_1939718218409672785[58] = 0.0;
   out_1939718218409672785[59] = 0.0;
   out_1939718218409672785[60] = 1.0;
   out_1939718218409672785[61] = 0.0;
   out_1939718218409672785[62] = 0.0;
   out_1939718218409672785[63] = 0.0;
   out_1939718218409672785[64] = 0.0;
   out_1939718218409672785[65] = 0.0;
   out_1939718218409672785[66] = 0.0;
   out_1939718218409672785[67] = 0.0;
   out_1939718218409672785[68] = 0.0;
   out_1939718218409672785[69] = 0.0;
   out_1939718218409672785[70] = 1.0;
   out_1939718218409672785[71] = 0.0;
   out_1939718218409672785[72] = 0.0;
   out_1939718218409672785[73] = 0.0;
   out_1939718218409672785[74] = 0.0;
   out_1939718218409672785[75] = 0.0;
   out_1939718218409672785[76] = 0.0;
   out_1939718218409672785[77] = 0.0;
   out_1939718218409672785[78] = 0.0;
   out_1939718218409672785[79] = 0.0;
   out_1939718218409672785[80] = 1.0;
}
void f_fun(double *state, double dt, double *out_3926727015556538206) {
   out_3926727015556538206[0] = state[0];
   out_3926727015556538206[1] = state[1];
   out_3926727015556538206[2] = state[2];
   out_3926727015556538206[3] = state[3];
   out_3926727015556538206[4] = state[4];
   out_3926727015556538206[5] = dt*((-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]))*state[6] - 9.8100000000000005*state[8] + stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*state[1]) + (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*state[4])) + state[5];
   out_3926727015556538206[6] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*state[4])) + state[6];
   out_3926727015556538206[7] = state[7];
   out_3926727015556538206[8] = state[8];
}
void F_fun(double *state, double dt, double *out_2236832154434567522) {
   out_2236832154434567522[0] = 1;
   out_2236832154434567522[1] = 0;
   out_2236832154434567522[2] = 0;
   out_2236832154434567522[3] = 0;
   out_2236832154434567522[4] = 0;
   out_2236832154434567522[5] = 0;
   out_2236832154434567522[6] = 0;
   out_2236832154434567522[7] = 0;
   out_2236832154434567522[8] = 0;
   out_2236832154434567522[9] = 0;
   out_2236832154434567522[10] = 1;
   out_2236832154434567522[11] = 0;
   out_2236832154434567522[12] = 0;
   out_2236832154434567522[13] = 0;
   out_2236832154434567522[14] = 0;
   out_2236832154434567522[15] = 0;
   out_2236832154434567522[16] = 0;
   out_2236832154434567522[17] = 0;
   out_2236832154434567522[18] = 0;
   out_2236832154434567522[19] = 0;
   out_2236832154434567522[20] = 1;
   out_2236832154434567522[21] = 0;
   out_2236832154434567522[22] = 0;
   out_2236832154434567522[23] = 0;
   out_2236832154434567522[24] = 0;
   out_2236832154434567522[25] = 0;
   out_2236832154434567522[26] = 0;
   out_2236832154434567522[27] = 0;
   out_2236832154434567522[28] = 0;
   out_2236832154434567522[29] = 0;
   out_2236832154434567522[30] = 1;
   out_2236832154434567522[31] = 0;
   out_2236832154434567522[32] = 0;
   out_2236832154434567522[33] = 0;
   out_2236832154434567522[34] = 0;
   out_2236832154434567522[35] = 0;
   out_2236832154434567522[36] = 0;
   out_2236832154434567522[37] = 0;
   out_2236832154434567522[38] = 0;
   out_2236832154434567522[39] = 0;
   out_2236832154434567522[40] = 1;
   out_2236832154434567522[41] = 0;
   out_2236832154434567522[42] = 0;
   out_2236832154434567522[43] = 0;
   out_2236832154434567522[44] = 0;
   out_2236832154434567522[45] = dt*(stiffness_front*(-state[2] - state[3] + state[7])/(mass*state[1]) + (-stiffness_front - stiffness_rear)*state[5]/(mass*state[4]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[6]/(mass*state[4]));
   out_2236832154434567522[46] = -dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*pow(state[1], 2));
   out_2236832154434567522[47] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_2236832154434567522[48] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_2236832154434567522[49] = dt*((-1 - (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*pow(state[4], 2)))*state[6] - (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*pow(state[4], 2)));
   out_2236832154434567522[50] = dt*(-stiffness_front*state[0] - stiffness_rear*state[0])/(mass*state[4]) + 1;
   out_2236832154434567522[51] = dt*(-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]));
   out_2236832154434567522[52] = dt*stiffness_front*state[0]/(mass*state[1]);
   out_2236832154434567522[53] = -9.8100000000000005*dt;
   out_2236832154434567522[54] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front - pow(center_to_rear, 2)*stiffness_rear)*state[6]/(rotational_inertia*state[4]));
   out_2236832154434567522[55] = -center_to_front*dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*pow(state[1], 2));
   out_2236832154434567522[56] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_2236832154434567522[57] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_2236832154434567522[58] = dt*(-(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*pow(state[4], 2)) - (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*pow(state[4], 2)));
   out_2236832154434567522[59] = dt*(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(rotational_inertia*state[4]);
   out_2236832154434567522[60] = dt*(-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])/(rotational_inertia*state[4]) + 1;
   out_2236832154434567522[61] = center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_2236832154434567522[62] = 0;
   out_2236832154434567522[63] = 0;
   out_2236832154434567522[64] = 0;
   out_2236832154434567522[65] = 0;
   out_2236832154434567522[66] = 0;
   out_2236832154434567522[67] = 0;
   out_2236832154434567522[68] = 0;
   out_2236832154434567522[69] = 0;
   out_2236832154434567522[70] = 1;
   out_2236832154434567522[71] = 0;
   out_2236832154434567522[72] = 0;
   out_2236832154434567522[73] = 0;
   out_2236832154434567522[74] = 0;
   out_2236832154434567522[75] = 0;
   out_2236832154434567522[76] = 0;
   out_2236832154434567522[77] = 0;
   out_2236832154434567522[78] = 0;
   out_2236832154434567522[79] = 0;
   out_2236832154434567522[80] = 1;
}
void h_25(double *state, double *unused, double *out_3928584535156480826) {
   out_3928584535156480826[0] = state[6];
}
void H_25(double *state, double *unused, double *out_6843410043594337625) {
   out_6843410043594337625[0] = 0;
   out_6843410043594337625[1] = 0;
   out_6843410043594337625[2] = 0;
   out_6843410043594337625[3] = 0;
   out_6843410043594337625[4] = 0;
   out_6843410043594337625[5] = 0;
   out_6843410043594337625[6] = 1;
   out_6843410043594337625[7] = 0;
   out_6843410043594337625[8] = 0;
}
void h_24(double *state, double *unused, double *out_3301920948150113080) {
   out_3301920948150113080[0] = state[4];
   out_3301920948150113080[1] = state[5];
}
void H_24(double *state, double *unused, double *out_599264549394378695) {
   out_599264549394378695[0] = 0;
   out_599264549394378695[1] = 0;
   out_599264549394378695[2] = 0;
   out_599264549394378695[3] = 0;
   out_599264549394378695[4] = 1;
   out_599264549394378695[5] = 0;
   out_599264549394378695[6] = 0;
   out_599264549394378695[7] = 0;
   out_599264549394378695[8] = 0;
   out_599264549394378695[9] = 0;
   out_599264549394378695[10] = 0;
   out_599264549394378695[11] = 0;
   out_599264549394378695[12] = 0;
   out_599264549394378695[13] = 0;
   out_599264549394378695[14] = 1;
   out_599264549394378695[15] = 0;
   out_599264549394378695[16] = 0;
   out_599264549394378695[17] = 0;
}
void h_30(double *state, double *unused, double *out_3653390472871974937) {
   out_3653390472871974937[0] = state[4];
}
void H_30(double *state, double *unused, double *out_2315713713466729427) {
   out_2315713713466729427[0] = 0;
   out_2315713713466729427[1] = 0;
   out_2315713713466729427[2] = 0;
   out_2315713713466729427[3] = 0;
   out_2315713713466729427[4] = 1;
   out_2315713713466729427[5] = 0;
   out_2315713713466729427[6] = 0;
   out_2315713713466729427[7] = 0;
   out_2315713713466729427[8] = 0;
}
void h_26(double *state, double *unused, double *out_2566663824515510610) {
   out_2566663824515510610[0] = state[7];
}
void H_26(double *state, double *unused, double *out_3101906724720281401) {
   out_3101906724720281401[0] = 0;
   out_3101906724720281401[1] = 0;
   out_3101906724720281401[2] = 0;
   out_3101906724720281401[3] = 0;
   out_3101906724720281401[4] = 0;
   out_3101906724720281401[5] = 0;
   out_3101906724720281401[6] = 0;
   out_3101906724720281401[7] = 1;
   out_3101906724720281401[8] = 0;
}
void h_27(double *state, double *unused, double *out_1949700952402234262) {
   out_1949700952402234262[0] = state[3];
}
void H_27(double *state, double *unused, double *out_140950401666304516) {
   out_140950401666304516[0] = 0;
   out_140950401666304516[1] = 0;
   out_140950401666304516[2] = 0;
   out_140950401666304516[3] = 1;
   out_140950401666304516[4] = 0;
   out_140950401666304516[5] = 0;
   out_140950401666304516[6] = 0;
   out_140950401666304516[7] = 0;
   out_140950401666304516[8] = 0;
}
void h_29(double *state, double *unused, double *out_8537528878878529425) {
   out_8537528878878529425[0] = state[1];
}
void H_29(double *state, double *unused, double *out_2825945057781121611) {
   out_2825945057781121611[0] = 0;
   out_2825945057781121611[1] = 1;
   out_2825945057781121611[2] = 0;
   out_2825945057781121611[3] = 0;
   out_2825945057781121611[4] = 0;
   out_2825945057781121611[5] = 0;
   out_2825945057781121611[6] = 0;
   out_2825945057781121611[7] = 0;
   out_2825945057781121611[8] = 0;
}
void h_28(double *state, double *unused, double *out_6571886981858342576) {
   out_6571886981858342576[0] = state[0];
}
void H_28(double *state, double *unused, double *out_4789575329346447862) {
   out_4789575329346447862[0] = 1;
   out_4789575329346447862[1] = 0;
   out_4789575329346447862[2] = 0;
   out_4789575329346447862[3] = 0;
   out_4789575329346447862[4] = 0;
   out_4789575329346447862[5] = 0;
   out_4789575329346447862[6] = 0;
   out_4789575329346447862[7] = 0;
   out_4789575329346447862[8] = 0;
}
void h_31(double *state, double *unused, double *out_4711292941562061058) {
   out_4711292941562061058[0] = state[8];
}
void H_31(double *state, double *unused, double *out_2475698622486929925) {
   out_2475698622486929925[0] = 0;
   out_2475698622486929925[1] = 0;
   out_2475698622486929925[2] = 0;
   out_2475698622486929925[3] = 0;
   out_2475698622486929925[4] = 0;
   out_2475698622486929925[5] = 0;
   out_2475698622486929925[6] = 0;
   out_2475698622486929925[7] = 0;
   out_2475698622486929925[8] = 1;
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
void car_err_fun(double *nom_x, double *delta_x, double *out_8235846690843231890) {
  err_fun(nom_x, delta_x, out_8235846690843231890);
}
void car_inv_err_fun(double *nom_x, double *true_x, double *out_4073537797462342510) {
  inv_err_fun(nom_x, true_x, out_4073537797462342510);
}
void car_H_mod_fun(double *state, double *out_1939718218409672785) {
  H_mod_fun(state, out_1939718218409672785);
}
void car_f_fun(double *state, double dt, double *out_3926727015556538206) {
  f_fun(state,  dt, out_3926727015556538206);
}
void car_F_fun(double *state, double dt, double *out_2236832154434567522) {
  F_fun(state,  dt, out_2236832154434567522);
}
void car_h_25(double *state, double *unused, double *out_3928584535156480826) {
  h_25(state, unused, out_3928584535156480826);
}
void car_H_25(double *state, double *unused, double *out_6843410043594337625) {
  H_25(state, unused, out_6843410043594337625);
}
void car_h_24(double *state, double *unused, double *out_3301920948150113080) {
  h_24(state, unused, out_3301920948150113080);
}
void car_H_24(double *state, double *unused, double *out_599264549394378695) {
  H_24(state, unused, out_599264549394378695);
}
void car_h_30(double *state, double *unused, double *out_3653390472871974937) {
  h_30(state, unused, out_3653390472871974937);
}
void car_H_30(double *state, double *unused, double *out_2315713713466729427) {
  H_30(state, unused, out_2315713713466729427);
}
void car_h_26(double *state, double *unused, double *out_2566663824515510610) {
  h_26(state, unused, out_2566663824515510610);
}
void car_H_26(double *state, double *unused, double *out_3101906724720281401) {
  H_26(state, unused, out_3101906724720281401);
}
void car_h_27(double *state, double *unused, double *out_1949700952402234262) {
  h_27(state, unused, out_1949700952402234262);
}
void car_H_27(double *state, double *unused, double *out_140950401666304516) {
  H_27(state, unused, out_140950401666304516);
}
void car_h_29(double *state, double *unused, double *out_8537528878878529425) {
  h_29(state, unused, out_8537528878878529425);
}
void car_H_29(double *state, double *unused, double *out_2825945057781121611) {
  H_29(state, unused, out_2825945057781121611);
}
void car_h_28(double *state, double *unused, double *out_6571886981858342576) {
  h_28(state, unused, out_6571886981858342576);
}
void car_H_28(double *state, double *unused, double *out_4789575329346447862) {
  H_28(state, unused, out_4789575329346447862);
}
void car_h_31(double *state, double *unused, double *out_4711292941562061058) {
  h_31(state, unused, out_4711292941562061058);
}
void car_H_31(double *state, double *unused, double *out_2475698622486929925) {
  H_31(state, unused, out_2475698622486929925);
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
