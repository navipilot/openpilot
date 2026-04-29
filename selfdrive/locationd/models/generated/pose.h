#pragma once
#include "rednose/helpers/ekf.h"
extern "C" {
void pose_update_4(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_10(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_13(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_14(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_err_fun(double *nom_x, double *delta_x, double *out_1903969700519405728);
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_3362098330798648840);
void pose_H_mod_fun(double *state, double *out_3822470159260117769);
void pose_f_fun(double *state, double dt, double *out_4539628665192259779);
void pose_F_fun(double *state, double dt, double *out_4279109943705499799);
void pose_h_4(double *state, double *unused, double *out_4468827944582346069);
void pose_H_4(double *state, double *unused, double *out_4576631161314830376);
void pose_h_10(double *state, double *unused, double *out_7687610123346311094);
void pose_H_10(double *state, double *unused, double *out_1094040887207712728);
void pose_h_13(double *state, double *unused, double *out_649951466656749498);
void pose_H_13(double *state, double *unused, double *out_6259481704078020311);
void pose_h_14(double *state, double *unused, double *out_5063684233320489319);
void pose_H_14(double *state, double *unused, double *out_8539872017654314905);
void pose_predict(double *in_x, double *in_P, double *in_Q, double dt);
}