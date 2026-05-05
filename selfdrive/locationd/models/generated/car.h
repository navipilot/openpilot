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
void car_err_fun(double *nom_x, double *delta_x, double *out_5457591731668657994);
void car_inv_err_fun(double *nom_x, double *true_x, double *out_625468362349040739);
void car_H_mod_fun(double *state, double *out_308877862146519903);
void car_f_fun(double *state, double dt, double *out_9068006394827680313);
void car_F_fun(double *state, double dt, double *out_203771605761279388);
void car_h_25(double *state, double *unused, double *out_5131794037626248505);
void car_H_25(double *state, double *unused, double *out_640243059881962306);
void car_h_24(double *state, double *unused, double *out_2029903564179396636);
void car_H_24(double *state, double *unused, double *out_2865950843860830868);
void car_h_30(double *state, double *unused, double *out_8651236171946894111);
void car_H_30(double *state, double *unused, double *out_510904112738722236);
void car_h_26(double *state, double *unused, double *out_2252194654533044965);
void car_H_26(double *state, double *unused, double *out_3101260258992093918);
void car_h_27(double *state, double *unused, double *out_3283700363150443165);
void car_H_27(double *state, double *unused, double *out_1663859199061702675);
void car_h_29(double *state, double *unused, double *out_3008506300865937276);
void car_H_29(double *state, double *unused, double *out_1021135457053114420);
void car_h_28(double *state, double *unused, double *out_5663871735784661514);
void car_H_28(double *state, double *unused, double *out_2984765728618440671);
void car_h_31(double *state, double *unused, double *out_7524465776080943032);
void car_H_31(double *state, double *unused, double *out_670889021758922734);
void car_predict(double *in_x, double *in_P, double *in_Q, double dt);
void car_set_mass(double x);
void car_set_rotational_inertia(double x);
void car_set_center_to_front(double x);
void car_set_center_to_rear(double x);
void car_set_stiffness_front(double x);
void car_set_stiffness_rear(double x);
}