#ifndef RUNNINGSTATS_HH
#define RUNNINGSTATS_HH

namespace fn
{
    //One pass computation of statistical moments, based on Knuth and Welford
    //Stolen from http://www.johndcook.com/skewness_kurtosis.html
    //Updated with http://en.wikipedia.org/wiki/
    //Algorithms_for_calculating_variance#Weighted_incremental_algorithm
    class RunningStats
    {
        public:
            RunningStats();
            void Clear();
            void Push(double x, double weight);
            double  TotalWeight() const;
            double Mean() const;
            double Variance() const;
            double StandardDeviation() const;

        private:
            double  sumweight;
            double M1, M2;
    };

}

#endif
