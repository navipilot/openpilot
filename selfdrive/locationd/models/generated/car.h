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
void car_err_fun(double *nom_x, double *delta_x, double *out_151033249954379814);
void car_inv_err_fun(double *nom_x, double *true_x, double *out_183614081703202987);
void car_H_mod_fun(double *state, double *out_7146229962937001843);
void car_f_fun(double *state, double dt, double *out_8130334584715691889);
void car_F_fun(double *state, double dt, double *out_2461222714691540299);
void car_h_25(double *state, double *unused, double *out_6813335027708603723);
void car_H_25(double *state, double *unused, double *out_2046958262975235394);
void car_h_24(double *state, double *unused, double *out_2151628798375541770);
void car_H_24(double *state, double *unused, double *out_6793467589941744428);
void car_h_30(double *state, double *unused, double *out_7995959517746925543);
void car_H_30(double *state, double *unused, double *out_4869732078516381361);
void car_h_26(double *state, double *unused, double *out_7052449664845263108);
void car_H_26(double *state, double *unused, double *out_5788461581849291618);
void car_h_27(double *state, double *unused, double *out_3776548976264271038);
void car_H_27(double *state, double *unused, double *out_2694968766715956450);
void car_h_29(double *state, double *unused, double *out_2994286250086079898);
void car_H_29(double *state, double *unused, double *out_981606039846405417);
void car_h_28(double *state, double *unused, double *out_8174501057545900050);
void car_H_28(double *state, double *unused, double *out_4100792977223125157);
void car_h_31(double *state, double *unused, double *out_378214085146908073);
void car_H_31(double *state, double *unused, double *out_2016312301098274966);
void car_predict(double *in_x, double *in_P, double *in_Q, double dt);
void car_set_mass(double x);
void car_set_rotational_inertia(double x);
void car_set_center_to_front(double x);
void car_set_center_to_rear(double x);
void car_set_stiffness_front(double x);
void car_set_stiffness_rear(double x);
}