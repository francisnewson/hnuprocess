#include "RunningStats.hh"
#include <cmath>
#include <vector>

namespace fn
{

    RunningStats::RunningStats() 
    {
        Clear();
    }

    void RunningStats::Clear()
    {
        sumweight = 0;
        M1 = 0;
        M2 = 0;
    }

    void RunningStats::Push(double x, double weight)
    {
        double temp = sumweight + weight;
        double delta = x - M1;
        double R = delta * weight / temp;

        M1 += R;
        M2 += sumweight * delta * R;
        sumweight = temp;
    }

    double RunningStats::TotalWeight() const
    {
        return sumweight;
    }

    double RunningStats::Mean() const
    {
        return M1;
    }

    double RunningStats::Variance() const
    {
        return M2/sumweight;
    }

    double RunningStats::StandardDeviation() const
    {
        return sqrt( Variance() );
    }

}
