#pragma once
#include "rednose/helpers/ekf.h"
extern "C" {
void car_update_25(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_24(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_30(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_26(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_27(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_29(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_28(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_31(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_err_fun(double *nom_x, double *delta_x, double *out_417238876910560070);
void car_inv_err_fun(double *nom_x, double *true_x, double *out_2214194102385980691);
void car_H_mod_fun(double *state, double *out_2617678226180198985);
void car_f_fun(double *state, double dt, double *out_2248561019141826344);
void car_F_fun(double *state, double dt, double *out_1589062328883606575);
void car_h_25(double *state, double *unused, double *out_4251901549604751950);
void car_H_25(double *state, double *unused, double *out_7412509069108068094);
void car_h_24(double *state, double *unused, double *out_6482324594163419455);
void car_H_24(double *state, double *unused, double *out_1815556116961127131);
void car_h_30(double *state, double *unused, double *out_3976707487320246061);
void car_H_30(double *state, double *unused, double *out_6506538674473875324);
void car_h_26(double *state, double *unused, double *out_8978380754981091706);
void car_H_26(double *state, double *unused, double *out_7292731685727427298);
void car_h_27(double *state, double *unused, double *out_6938698807011347957);
void car_H_27(double *state, double *unused, double *out_4331775362673450413);
void car_h_29(double *state, double *unused, double *out_4741129432548446265);
void car_H_29(double *state, double *unused, double *out_7016770018788267508);
void car_h_28(double *state, double *unused, double *out_64063896781717822);
void car_H_28(double *state, double *unused, double *out_8980400290353593759);
void car_h_31(double *state, double *unused, double *out_4885815735079099232);
void car_H_31(double *state, double *unused, double *out_6666523583494075822);
void car_predict(double *in_x, double *in_P, double *in_Q, double dt);
void car_set_mass(double x);
void car_set_rotational_inertia(double x);
void car_set_center_to_front(double x);
void car_set_center_to_rear(double x);
void car_set_stiffness_front(double x);
void car_set_stiffness_rear(double x);
}