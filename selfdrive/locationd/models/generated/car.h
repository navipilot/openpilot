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
void car_err_fun(double *nom_x, double *delta_x, double *out_1461273597009064329);
void car_inv_err_fun(double *nom_x, double *true_x, double *out_7289765730580897738);
void car_H_mod_fun(double *state, double *out_8304527178040913025);
void car_f_fun(double *state, double dt, double *out_1355173870151422192);
void car_F_fun(double *state, double dt, double *out_3219998669912464347);
void car_h_25(double *state, double *unused, double *out_7686598041594365016);
void car_H_25(double *state, double *unused, double *out_2220878862694778786);
void car_h_24(double *state, double *unused, double *out_3813839643785118531);
void car_H_24(double *state, double *unused, double *out_1337046736107544350);
void car_h_30(double *state, double *unused, double *out_2120268555396644716);
void car_H_30(double *state, double *unused, double *out_297454095812469841);
void car_h_26(double *state, double *unused, double *out_2881404712264200929);
void car_H_26(double *state, double *unused, double *out_5962382181568835010);
void car_h_27(double *state, double *unused, double *out_901564640210476392);
void car_H_27(double *state, double *unused, double *out_2521048166996413058);
void car_h_29(double *state, double *unused, double *out_952524754320502979);
void car_H_29(double *state, double *unused, double *out_807685440126862025);
void car_h_28(double *state, double *unused, double *out_8550909813521810734);
void car_H_28(double *state, double *unused, double *out_4274713576942668549);
void car_h_31(double *state, double *unused, double *out_7961792103878870905);
void car_H_31(double *state, double *unused, double *out_6588590283802186486);
void car_predict(double *in_x, double *in_P, double *in_Q, double dt);
void car_set_mass(double x);
void car_set_rotational_inertia(double x);
void car_set_center_to_front(double x);
void car_set_center_to_rear(double x);
void car_set_stiffness_front(double x);
void car_set_stiffness_rear(double x);
}