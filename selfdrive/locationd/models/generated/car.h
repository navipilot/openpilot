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
void car_err_fun(double *nom_x, double *delta_x, double *out_3525274203873677771);
void car_inv_err_fun(double *nom_x, double *true_x, double *out_297560133990373422);
void car_H_mod_fun(double *state, double *out_3681855867780802793);
void car_f_fun(double *state, double dt, double *out_1710696929836261316);
void car_F_fun(double *state, double dt, double *out_1799025913951617520);
void car_h_25(double *state, double *unused, double *out_5416149830362286688);
void car_H_25(double *state, double *unused, double *out_1045298942765376599);
void car_h_24(double *state, double *unused, double *out_7084306918088164219);
void car_H_24(double *state, double *unused, double *out_8188902388611608138);
void car_h_30(double *state, double *unused, double *out_5691343892646792577);
void car_H_30(double *state, double *unused, double *out_3482397387362231599);
void car_h_26(double *state, double *unused, double *out_7270316149721926584);
void car_H_26(double *state, double *unused, double *out_2696204376108679625);
void car_h_27(double *state, double *unused, double *out_341835083631451268);
void car_H_27(double *state, double *unused, double *out_1258803316178288382);
void car_h_29(double *state, double *unused, double *out_7112670309981802204);
void car_H_29(double *state, double *unused, double *out_2972166043047839415);
void car_h_28(double *state, double *unused, double *out_2530122726332441526);
void car_H_28(double *state, double *unused, double *out_8054565060117369989);
void car_h_31(double *state, double *unused, double *out_2419756905528375528);
void car_H_31(double *state, double *unused, double *out_1075944904642337027);
void car_predict(double *in_x, double *in_P, double *in_Q, double dt);
void car_set_mass(double x);
void car_set_rotational_inertia(double x);
void car_set_center_to_front(double x);
void car_set_center_to_rear(double x);
void car_set_stiffness_front(double x);
void car_set_stiffness_rear(double x);
}