#pragma once
#include "rednose/helpers/ekf.h"
extern "C" {
void pose_update_4(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_10(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_13(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_14(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_err_fun(double *nom_x, double *delta_x, double *out_5984861409348236479);
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_1707565234864444406);
void pose_H_mod_fun(double *state, double *out_938651669609787352);
void pose_f_fun(double *state, double dt, double *out_336566407888546136);
void pose_F_fun(double *state, double dt, double *out_5996324224975557063);
void pose_h_4(double *state, double *unused, double *out_2660879566845649022);
void pose_H_4(double *state, double *unused, double *out_6786733476361746073);
void pose_h_10(double *state, double *unused, double *out_323021619404120390);
void pose_H_10(double *state, double *unused, double *out_4140593631986057126);
void pose_h_13(double *state, double *unused, double *out_3801171022701447110);
void pose_H_13(double *state, double *unused, double *out_3574459651029413272);
void pose_h_14(double *state, double *unused, double *out_7703097660121677315);
void pose_H_14(double *state, double *unused, double *out_2823492620022261544);
void pose_predict(double *in_x, double *in_P, double *in_Q, double dt);
}