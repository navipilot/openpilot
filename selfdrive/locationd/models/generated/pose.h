#pragma once
#include "rednose/helpers/ekf.h"
extern "C" {
void pose_update_4(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_10(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_13(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_14(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_err_fun(double *nom_x, double *delta_x, double *out_9191542510053319957);
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_3004969772891744590);
void pose_H_mod_fun(double *state, double *out_7359357434108742810);
void pose_f_fun(double *state, double dt, double *out_729710836783527414);
void pose_F_fun(double *state, double dt, double *out_8499021045100499940);
void pose_h_4(double *state, double *unused, double *out_1075969755128808186);
void pose_H_4(double *state, double *unused, double *out_5239304832848850085);
void pose_h_10(double *state, double *unused, double *out_4050058798394118968);
void pose_H_10(double *state, double *unused, double *out_3614267989921698938);
void pose_h_13(double *state, double *unused, double *out_9106371473023855959);
void pose_H_13(double *state, double *unused, double *out_8451578658181182886);
void pose_h_14(double *state, double *unused, double *out_4347596682095509082);
void pose_H_14(double *state, double *unused, double *out_9202545689188334614);
void pose_predict(double *in_x, double *in_P, double *in_Q, double dt);
}