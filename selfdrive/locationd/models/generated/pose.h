#pragma once
#include "rednose/helpers/ekf.h"
extern "C" {
void pose_update_4(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_10(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_13(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_14(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_err_fun(double *nom_x, double *delta_x, double *out_3212432162194349052);
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_8973271523308122061);
void pose_H_mod_fun(double *state, double *out_7731698092980355432);
void pose_f_fun(double *state, double dt, double *out_8923016311880444304);
void pose_F_fun(double *state, double dt, double *out_210116950988546947);
void pose_h_4(double *state, double *unused, double *out_7648624411257127562);
void pose_H_4(double *state, double *unused, double *out_3959890883136447860);
void pose_h_10(double *state, double *unused, double *out_6099133755902511145);
void pose_H_10(double *state, double *unused, double *out_2569809615037315587);
void pose_h_13(double *state, double *unused, double *out_2653699383913433253);
void pose_H_13(double *state, double *unused, double *out_747617057804115059);
void pose_h_14(double *state, double *unused, double *out_7344654912082976031);
void pose_H_14(double *state, double *unused, double *out_3349973203036669);
void pose_predict(double *in_x, double *in_P, double *in_Q, double dt);
}