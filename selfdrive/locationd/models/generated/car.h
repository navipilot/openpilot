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
void car_err_fun(double *nom_x, double *delta_x, double *out_2165520225426293203);
void car_inv_err_fun(double *nom_x, double *true_x, double *out_48394968560250279);
void car_H_mod_fun(double *state, double *out_8198135828365669542);
void car_f_fun(double *state, double dt, double *out_5413666688161130858);
void car_F_fun(double *state, double dt, double *out_7314745635201977822);
void car_h_25(double *state, double *unused, double *out_1723156805424676224);
void car_H_25(double *state, double *unused, double *out_6725559606698516926);
void car_h_24(double *state, double *unused, double *out_6301088908339500348);
void car_H_24(double *state, double *unused, double *out_8163258868208711554);
void car_h_30(double *state, double *unused, double *out_5821460103704048088);
void car_H_30(double *state, double *unused, double *out_4804494125519417935);
void car_h_26(double *state, double *unused, double *out_3604740557565027086);
void car_H_26(double *state, double *unused, double *out_8416658497250234089);
void car_h_27(double *state, double *unused, double *out_4759942856869008909);
void car_H_27(double *state, double *unused, double *out_6979257437319842846);
void car_h_29(double *state, double *unused, double *out_5218144219027570571);
void car_H_29(double *state, double *unused, double *out_4294262781205025751);
void car_h_28(double *state, double *unused, double *out_5703452709770671344);
void car_H_28(double *state, double *unused, double *out_4671724892450627163);
void car_h_31(double *state, double *unused, double *out_1998350867709182113);
void car_H_31(double *state, double *unused, double *out_4644509216499217437);
void car_predict(double *in_x, double *in_P, double *in_Q, double dt);
void car_set_mass(double x);
void car_set_rotational_inertia(double x);
void car_set_center_to_front(double x);
void car_set_center_to_rear(double x);
void car_set_stiffness_front(double x);
void car_set_stiffness_rear(double x);
}