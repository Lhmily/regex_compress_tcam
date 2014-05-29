/*
 * leakage_function.h
 *
 *  Created on: May 25, 2014
 *      Author: Lhmily
 */

#ifndef LEAKAGE_FUNCTION_H_
#define LEAKAGE_FUNCTION_H_


/* Technology Length */

double nmos_ileakage(double aspect_ratio, double Volt, double Vth0,
		double Tkelvin, double tox0);

double pmos_ileakage(double aspect_ratio, double Volt, double Vth0,
		double Tkelvin, double tox0);

double nmos_ileakage_var(double aspect_ratio, double Volt, double Vth0,
		double Tkelvin, double tox0, double tech_length);

double pmos_ileakage_var(double aspect_ratio, double Volt, double Vth0,
		double Tkelvin, double tox0, double tech_length);

double box_mueller(double std_var, double value);

double simplified_cmos_leakage(double naspect_ratio, double paspect_ratio,
		double nVth0, double pVth0, double *norm_nleak, double *norm_pleak);

double simplified_nmos_leakage(double naspect_ratio, double nVth0);
double simplified_pmos_leakage(double paspect_ratio, double pVth0);
double cmos_ileakage(double nWidth, double pWidth, double nVthreshold,
		double pVthreshold);

void precalc_leakage_params(double Volt, double Tkelvin, double tox0,
		double tech_length);

void init_leakage_params(double tech);

#endif /* LEAKAGE_FUNCTION_H_ */
