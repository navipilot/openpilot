#pragma once
#include "rednose/helpers/ekf.h"
extern "C" {
void pose_update_4(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_10(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_13(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_14(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_err_fun(double *nom_x, double *delta_x, double *out_6656022622757342812);
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_5165337953826908445);
void pose_H_mod_fun(double *state, double *out_5190446458913695382);
void pose_f_fun(double *state, double dt, double *out_337750480546902101);
void pose_F_fun(double *state, double dt, double *out_1252525263334667707);
void pose_h_4(double *state, double *unused, double *out_1011967617642247994);
void pose_H_4(double *state, double *unused, double *out_7408215808043897513);
void pose_h_10(double *state, double *unused, double *out_511164501586373249);
void pose_H_10(double *state, double *unused, double *out_8712555441471564813);
void pose_h_13(double *state, double *unused, double *out_6347656539957505651);
void pose_H_13(double *state, double *unused, double *out_7826254440333321302);
void pose_h_14(double *state, double *unused, double *out_251978956082886222);
void pose_H_14(double *state, double *unused, double *out_7075287409326169574);
void pose_predict(double *in_x, double *in_P, double *in_Q, double dt);
}