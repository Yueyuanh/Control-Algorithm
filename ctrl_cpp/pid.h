#pragma once
#include "utils.h"

//PID������
class PIDController {
// ����������
private:
    // PID����
    vector<double> __Kp;
    vector<double> __Ki;
    vector<double> __Kd;
    vector<pair<double, double>> __control_limits; // ����Լ��
    // �Ƚ�PID����
    vector<double> __Kf; // ǰ������
    vector<double> __Kaw; // �����Ͳ���
    vector<pair<double, double>> __error_clip; // ����Լ��
    // ��������
    vector<double> __last_error; // ��һʱ�����
    vector<double> __integral_error; // ������
    vector<double> __u; // �������


// ���������������
public:
    PIDController(
        double Kp, 
        double Ki, 
        double Kd,
        const pair<double, double>& control_limits = { -Infinity, Infinity },
        double Kf = 0.0,
        double Kaw = 0.2,
        const pair<double, double>& error_clip = { -Infinity, Infinity }
    );
    /*
    :param Kp: float | list, ��������.
    :param Ki: float | list, ��������.
    :param Kd: float | list, ΢������.
    :param control_limits: list[float] | list[list[float]], ������Լ��, Ĭ��None��ʾ��Լ��.
    :param Kf: float | list, ǰ����������, Ĭ��0.
    :param Kaw: float | list, �����ֱ��Ͳ���, ���ȡ0.1~0.3, Ĭ��0.2, ȡ�㲻�����ֱ���.
    :param error_clip: float | list, list[float] | list[list[float]], ������������ֵ, ��Χ: (0, inf], ȡinfʱ�����������.
    */

    PIDController(
        const vector<double>& Kp, 
        const vector<double>& Ki, 
        const vector<double>& Kd,
        const vector<pair<double, double>>& control_limits = {},
        const vector<double>& Kf = {},
        const vector<double>& Kaw = {},
        const vector<pair<double, double>>& error_clip = {}
    );
    
    double call(double setpoint, double feedback, double dt, double expected_output = 0.0, int anti_windup_method = 1);
    /*
    :param setpoint: float | list, �ο��켣.
    :param feedback: float | list, ϵͳ���.
    :param dt: float, ϵͳ���ò���.
    :param expected_output: float | list, ����ϵͳ���, Ĭ��û��.
    :param anti_windup_method: Literal[0, 1], �����ֱ��ͷ���, Ĭ��1.
    */
    double operator()(double setpoint, double feedback, double dt, double expected_output = 0.0, int anti_windup_method = 1);
    vector<double> call(const vector<double>& setpoint, const vector<double>& feedback, double dt, const vector<double>& expected_output = {}, int anti_windup_method = 1);
    vector<double> operator()(const vector<double>& setpoint, const vector<double>& feedback, double dt, const vector<double>& expected_output = {}, int anti_windup_method = 1);


// @classmethod
private:
    static double _AntiWindup(double Ki, double& ierror, double error, pair<double, double> error_lim, double u, pair<double, double> u_lim, double Kaw, double dt, int method = 1); // ���ػ�����
    /* integral_error��������, integral�ǻ�����, ���ػ�����, ͬʱ������������ */
};
