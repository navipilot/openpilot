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
void car_err_fun(double *nom_x, double *delta_x, double *out_411609516403737386);
void car_inv_err_fun(double *nom_x, double *true_x, double *out_7082078735183717431);
void car_H_mod_fun(double *state, double *out_1390740467130861487);
void car_f_fun(double *state, double dt, double *out_2246396216540142177);
void car_F_fun(double *state, double dt, double *out_3263326103915417388);
void car_h_25(double *state, double *unused, double *out_7774150008233015132);
void car_H_25(double *state, double *unused, double *out_1594416823404739290);
void car_h_24(double *state, double *unused, double *out_3517564190390228050);
void car_H_24(double *state, double *unused, double *out_5823974810272187603);
void car_h_30(double *state, double *unused, double *out_3901758839126185548);
void car_H_30(double *state, double *unused, double *out_923916135102509337);
void car_h_26(double *state, double *unused, double *out_4894550625139811592);
void car_H_26(double *state, double *unused, double *out_5335920142278795514);
void car_h_27(double *state, double *unused, double *out_8717659861134677567);
void car_H_27(double *state, double *unused, double *out_1250847176697915574);
void car_h_29(double *state, double *unused, double *out_1089206735856332165);
void car_H_29(double *state, double *unused, double *out_1434147479416901521);
void car_h_28(double *state, double *unused, double *out_3715986593473831922);
void car_H_28(double *state, double *unused, double *out_3648251537652629053);
void car_h_31(double *state, double *unused, double *out_10884751412342691);
void car_H_31(double *state, double *unused, double *out_5962128244512146990);
void car_predict(double *in_x, double *in_P, double *in_Q, double dt);
void car_set_mass(double x);
void car_set_rotational_inertia(double x);
void car_set_center_to_front(double x);
void car_set_center_to_rear(double x);
void car_set_stiffness_front(double x);
void car_set_stiffness_rear(double x);
}