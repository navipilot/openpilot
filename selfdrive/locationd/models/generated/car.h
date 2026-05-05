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
void car_err_fun(double *nom_x, double *delta_x, double *out_1697783337874095353);
void car_inv_err_fun(double *nom_x, double *true_x, double *out_3236445373879696441);
void car_H_mod_fun(double *state, double *out_4178247856660158459);
void car_f_fun(double *state, double dt, double *out_6661857716321963649);
void car_F_fun(double *state, double dt, double *out_4395736809983768071);
void car_h_25(double *state, double *unused, double *out_2333658162275422577);
void car_H_25(double *state, double *unused, double *out_725443968524506381);
void car_h_24(double *state, double *unused, double *out_7377061376079036481);
void car_H_24(double *state, double *unused, double *out_1570440577308201780);
void car_h_30(double *state, double *unused, double *out_2058464099990916688);
void car_H_30(double *state, double *unused, double *out_3243776927031755008);
void car_h_26(double *state, double *unused, double *out_8738934800407300586);
void car_H_26(double *state, double *unused, double *out_3016059350349549843);
void car_h_27(double *state, double *unused, double *out_3277168015177085012);
void car_H_27(double *state, double *unused, double *out_5467370998215698225);
void car_h_29(double *state, double *unused, double *out_4679132473327541980);
void car_H_29(double *state, double *unused, double *out_3754008271346147192);
void car_h_28(double *state, double *unused, double *out_1724505252483760633);
void car_H_28(double *state, double *unused, double *out_1328390745723383382);
void car_h_31(double *state, double *unused, double *out_6530453262617598609);
void car_H_31(double *state, double *unused, double *out_756089930401466809);
void car_predict(double *in_x, double *in_P, double *in_Q, double dt);
void car_set_mass(double x);
void car_set_rotational_inertia(double x);
void car_set_center_to_front(double x);
void car_set_center_to_rear(double x);
void car_set_stiffness_front(double x);
void car_set_stiffness_rear(double x);
}