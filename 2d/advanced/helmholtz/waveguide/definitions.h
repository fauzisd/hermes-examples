#include "hermes2d.h"


/* Namespaces used */

using namespace Hermes;
using namespace Hermes::Hermes2D;
using namespace Hermes::Hermes2D::Views;
using namespace Hermes::Hermes2D::RefinementSelectors;


class EssentialBCNonConst : public EssentialBoundaryCondition<double> 
{
public:
    EssentialBCNonConst(std::string marker);

    ~EssentialBCNonConst() {};

    virtual EssentialBoundaryCondition<double>::EssentialBCValueType get_value_type() const;

    virtual double value(double x, double y, double n_x, double n_y, double t_x, double t_y) const;
};

class WeakFormHelmholtz : public WeakForm<double>
{
public:
    // Problems parameters
    WeakFormHelmholtz(double eps, double mu, double omega, double sigma, double beta, double E0, double h);

private:
    class MatrixFormHelmholtzEquation_real_real : public MatrixFormVol<double>
    {
    public:
        MatrixFormHelmholtzEquation_real_real(int i, int j, double eps, double omega, double mu)
              : MatrixFormVol<double>(i, j), eps(eps), omega(omega), mu(mu) {};

        template<typename Real, typename Scalar>
        Scalar matrix_form(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext) const;

        virtual double value(int n, double *wt, Func<double> *u_ext[], Func<double> *u, Func<double> *v, Geom<double> *e, ExtData<double> *ext) const;

        virtual Ord ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *u, Func<Ord> *v, Geom<Ord> *e, ExtData<Ord> *ext) const;

    private:
        // Memebers.
        double eps;
        double omega;
        double mu;
    };

    class MatrixFormHelmholtzEquation_real_imag : public MatrixFormVol<double>
    {
    private:
        // Memebers.
        double mu;
        double omega;
        double sigma;

    public:
        MatrixFormHelmholtzEquation_real_imag(int i, int j, double mu, double omega, double sigma) 
              : MatrixFormVol<double>(i, j), mu(mu), omega(omega), sigma(sigma) {};

        template<typename Real, typename Scalar>
        Scalar matrix_form(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext) const;

        virtual double value(int n, double *wt, Func<double> *u_ext[], Func<double> *u, Func<double> *v, Geom<double> *e, ExtData<double> *ext) const;

        virtual Ord ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *u, Func<Ord> *v, Geom<Ord> *e, ExtData<Ord> *ext) const;
    };

    class MatrixFormHelmholtzEquation_imag_real : public MatrixFormVol<double>
    {
    private:
        // Memebers.
        double mu;
        double omega;
        double sigma;

    public:
        MatrixFormHelmholtzEquation_imag_real(int i, int j, double mu, double omega, double sigma) 
              : MatrixFormVol<double>(i, j), mu(mu), omega(omega), sigma(sigma) {};

        template<typename Real, typename Scalar>
        Scalar matrix_form(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext) const;

        virtual double value(int n, double *wt, Func<double> *u_ext[], Func<double> *u, Func<double> *v, Geom<double> *e, ExtData<double> *ext) const;

        virtual Ord ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *u, Func<Ord> *v, Geom<Ord> *e, ExtData<Ord> *ext) const;
    };

    class MatrixFormHelmholtzEquation_imag_imag : public MatrixFormVol<double>
    {
    public:
        MatrixFormHelmholtzEquation_imag_imag(int i, int j, double eps, double mu, double omega) 
              : MatrixFormVol<double>(i, j), eps(eps), mu(mu), omega(omega) {};

        template<typename Real, typename Scalar>
        Scalar matrix_form(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext) const;

        virtual double value(int n, double *wt, Func<double> *u_ext[], Func<double> *u, Func<double> *v, Geom<double> *e, ExtData<double> *ext) const;

        virtual Ord ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *u, Func<Ord> *v, Geom<Ord> *e, ExtData<Ord> *ext) const;

        // Memebers.
        double eps;
        double mu;
        double omega;
    };


    class MatrixFormSurfHelmholtz_real_imag : public MatrixFormSurf<double>
    {
    private:
        double beta;
    public:
        MatrixFormSurfHelmholtz_real_imag(int i, int j, std::string area, double beta)
              : MatrixFormSurf<double>(i, j, area), beta(beta){};

        template<typename Real, typename Scalar>
        Scalar matrix_form_surf(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext) const;

        virtual double value(int n, double *wt, Func<double> *u_ext[], Func<double> *u, Func<double> *v, Geom<double> *e, ExtData<double> *ext) const;

        virtual Ord ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *u, Func<Ord> *v, Geom<Ord> *e, ExtData<Ord> *ext) const;
    };



    class MatrixFormSurfHelmholtz_imag_real : public MatrixFormSurf<double>
    {

    private:
        double beta;
    public:
        MatrixFormSurfHelmholtz_imag_real(int i, int j, std::string area, double beta)
              : MatrixFormSurf<double>(i, j, area), beta(beta){};

        template<typename Real, typename Scalar>
        Scalar matrix_form_surf(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext) const;

        virtual double value(int n, double *wt, Func<double> *u_ext[], Func<double> *u, Func<double> *v, Geom<double> *e, ExtData<double> *ext) const;

        virtual Ord ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *u, Func<Ord> *v, Geom<Ord> *e, ExtData<Ord> *ext) const;
    };
};
