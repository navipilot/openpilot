#pragma once
#include "rednose/helpers/ekf.h"
extern "C" {
void pose_update_4(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_10(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_13(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_14(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_err_fun(double *nom_x, double *delta_x, double *out_4940663026929446750);
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_1207218016783825442);
void pose_H_mod_fun(double *state, double *out_2820660951987973699);
void pose_f_fun(double *state, double dt, double *out_2310584025056258416);
void pose_F_fun(double *state, double dt, double *out_6557190266781458971);
void pose_h_4(double *state, double *unused, double *out_8177424571477984765);
void pose_H_4(double *state, double *unused, double *out_6592468161831881271);
void pose_h_10(double *state, double *unused, double *out_4220186743788310408);
void pose_H_10(double *state, double *unused, double *out_2494414403690571241);
void pose_h_13(double *state, double *unused, double *out_5780074361933493057);
void pose_H_13(double *state, double *unused, double *out_8642002086545337544);
void pose_h_14(double *state, double *unused, double *out_3473232613379301184);
void pose_H_14(double *state, double *unused, double *out_3509679729536508975);
void pose_predict(double *in_x, double *in_P, double *in_Q, double dt);
}