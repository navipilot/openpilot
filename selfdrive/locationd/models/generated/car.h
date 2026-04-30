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
void car_err_fun(double *nom_x, double *delta_x, double *out_8235846690843231890);
void car_inv_err_fun(double *nom_x, double *true_x, double *out_4073537797462342510);
void car_H_mod_fun(double *state, double *out_1939718218409672785);
void car_f_fun(double *state, double dt, double *out_3926727015556538206);
void car_F_fun(double *state, double dt, double *out_2236832154434567522);
void car_h_25(double *state, double *unused, double *out_3928584535156480826);
void car_H_25(double *state, double *unused, double *out_6843410043594337625);
void car_h_24(double *state, double *unused, double *out_3301920948150113080);
void car_H_24(double *state, double *unused, double *out_599264549394378695);
void car_h_30(double *state, double *unused, double *out_3653390472871974937);
void car_H_30(double *state, double *unused, double *out_2315713713466729427);
void car_h_26(double *state, double *unused, double *out_2566663824515510610);
void car_H_26(double *state, double *unused, double *out_3101906724720281401);
void car_h_27(double *state, double *unused, double *out_1949700952402234262);
void car_H_27(double *state, double *unused, double *out_140950401666304516);
void car_h_29(double *state, double *unused, double *out_8537528878878529425);
void car_H_29(double *state, double *unused, double *out_2825945057781121611);
void car_h_28(double *state, double *unused, double *out_6571886981858342576);
void car_H_28(double *state, double *unused, double *out_4789575329346447862);
void car_h_31(double *state, double *unused, double *out_4711292941562061058);
void car_H_31(double *state, double *unused, double *out_2475698622486929925);
void car_predict(double *in_x, double *in_P, double *in_Q, double dt);
void car_set_mass(double x);
void car_set_rotational_inertia(double x);
void car_set_center_to_front(double x);
void car_set_center_to_rear(double x);
void car_set_stiffness_front(double x);
void car_set_stiffness_rear(double x);
}