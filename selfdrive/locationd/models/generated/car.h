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
void car_err_fun(double *nom_x, double *delta_x, double *out_8994217345304474551);
void car_inv_err_fun(double *nom_x, double *true_x, double *out_8635883422281677209);
void car_H_mod_fun(double *state, double *out_5835492163818982048);
void car_f_fun(double *state, double dt, double *out_9046400166044584491);
void car_F_fun(double *state, double dt, double *out_8585883916966185499);
void car_h_25(double *state, double *unused, double *out_1895365982627284096);
void car_H_25(double *state, double *unused, double *out_3357696062093255189);
void car_h_24(double *state, double *unused, double *out_5360338391580302842);
void car_H_24(double *state, double *unused, double *out_5530345661098754755);
void car_h_30(double *state, double *unused, double *out_2052552650978413241);
void car_H_30(double *state, double *unused, double *out_839363103586006562);
void car_h_26(double *state, double *unused, double *out_2655868535654890758);
void car_H_26(double *state, double *unused, double *out_53170092332454588);
void car_h_27(double *state, double *unused, double *out_3774772722470275260);
void car_H_27(double *state, double *unused, double *out_1384230967597936655);
void car_h_29(double *state, double *unused, double *out_4656957971787110892);
void car_H_29(double *state, double *unused, double *out_329131759271614378);
void car_h_28(double *state, double *unused, double *out_1170367401661577609);
void car_H_28(double *state, double *unused, double *out_5411530776341144952);
void car_h_31(double *state, double *unused, double *out_1437164620468722434);
void car_H_31(double *state, double *unused, double *out_7725407483200662889);
void car_predict(double *in_x, double *in_P, double *in_Q, double dt);
void car_set_mass(double x);
void car_set_rotational_inertia(double x);
void car_set_center_to_front(double x);
void car_set_center_to_rear(double x);
void car_set_stiffness_front(double x);
void car_set_stiffness_rear(double x);
}