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
void car_err_fun(double *nom_x, double *delta_x, double *out_2687587277675065552);
void car_inv_err_fun(double *nom_x, double *true_x, double *out_6222700140799525089);
void car_H_mod_fun(double *state, double *out_5736311190330935470);
void car_f_fun(double *state, double dt, double *out_8059955085909907268);
void car_F_fun(double *state, double dt, double *out_535217138807217635);
void car_h_25(double *state, double *unused, double *out_7527307434081215851);
void car_H_25(double *state, double *unused, double *out_9187452233389124469);
void car_h_24(double *state, double *unused, double *out_7476062049775574580);
void car_H_24(double *state, double *unused, double *out_5629526634586801333);
void car_h_30(double *state, double *unused, double *out_1371860238240555722);
void car_H_30(double *state, double *unused, double *out_2270761891897507714);
void car_h_26(double *state, double *unused, double *out_6063283196188145091);
void car_H_26(double *state, double *unused, double *out_5517788521446370923);
void car_h_27(double *state, double *unused, double *out_8470817286982878286);
void car_H_27(double *state, double *unused, double *out_4445525203697932625);
void car_h_29(double *state, double *unused, double *out_7424449226025441418);
void car_H_29(double *state, double *unused, double *out_1760530547583115530);
void car_h_28(double *state, double *unused, double *out_2540783352297972517);
void car_H_28(double *state, double *unused, double *out_6842929564652646104);
void car_h_31(double *state, double *unused, double *out_7802501496365721740);
void car_H_31(double *state, double *unused, double *out_9156806271512164041);
void car_predict(double *in_x, double *in_P, double *in_Q, double dt);
void car_set_mass(double x);
void car_set_rotational_inertia(double x);
void car_set_center_to_front(double x);
void car_set_center_to_rear(double x);
void car_set_stiffness_front(double x);
void car_set_stiffness_rear(double x);
}