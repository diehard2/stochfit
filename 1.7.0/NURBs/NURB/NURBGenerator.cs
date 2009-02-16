using System;
using System.Collections.Generic;
using System.Text;

namespace NURB
{
    class ControlPoints
    {
        private float x;
        private float y;
        private float z;
        private float weights;

        public float GetWeights
        {
            get { return weights; }
            set { weights = value; }
        }

        public float GetZ
        {
            get { return z; }
            set { z = value; }
        }


        public float GetY
        {
            get { return y; }
            set { y = value; }
        }

        public float GetX
        {
            get { return x; }
            set { x = value; }
        }


        public ControlPoints(float cx, float cy, float cz, float cweights)
        {
            x = cx;
            y = cy;
            z = cz;
            weights = cweights;
        }

        public ControlPoints(float cx, float cy, float cz)
        {
            x = cx;
            y = cy;
            z = cz;
            weights = 1.0f;
        }
    }

    class NURBGenerator
    {
        float[] m_fKnotVector;
       
        /// <summary>
        /// Sets up the Knot Vector to pass to the Cox De-Boor algorithm
        /// </summary>
        /// <param name="p">number of control points</param>
        /// <param name="degree">degree of the curve</param>
        public void InitKnotVector(int p, int degree)
        {
            m_fKnotVector = new float[p + degree + 1];

            for (int i = 0; i < degree; i++)
                m_fKnotVector[i] = 0.0f;

            for (int i = degree + 1; i < m_fKnotVector.Length - (degree + 1); i++)
                m_fKnotVector[i] = ((float)i - (float)degree) / ((float)p - 1 - (float)degree + 1);

            for (int i = m_fKnotVector.Length - (degree + 1); i < m_fKnotVector.Length; i++)
                m_fKnotVector[i] = 1.0f;
        }


        /// <summary>
        /// Calculates the control points over a given length
        /// </summary>
        /// <param name="m_CP">array of control points</param>
        /// <param name="p">number of control points</param>
        /// <param name="length">total length from x = 0 to length</param>
        public void InitControlPoints(ref ControlPoints[] m_CP, int p, int length)
        {
            m_CP = new ControlPoints[p];
            float x = 0;
            Random rand = new Random();

            //Initialize
            for (int i = 0; i < p; i++)
            {
                x = (i * (float)length) / (((float)p - 1));
                m_CP[i] = new ControlPoints(x, (float)rand.NextDouble() * 100, 0, 1);
            }
        }

        /// <summary>
        /// Calculate the spline point for a given t
        /// </summary>
        /// <param name="t">The transformed coordinate</param>
        /// <param name="x">The x component in Euclidean space</param>
        /// <param name="y">The y component in Euclidean space</param>
        /// <param name="z">The z component in Euclidean space</param>
        /// <param name="CP">The control points for the spline</param>
        /// <param name="degree">The degree of the spline</param>
        public void GetOutpoint(float t, ref double x, ref double y, ref double z, ControlPoints[] CP, int degree)
        {

            // sum the effect of all CV's on the curve at this point to 
            // get the evaluated curve point
            // 

            float[] Val = new float[CP.Length];
            float ValSum = 0;

            for (int i = 0; i < CP.Length; i++)
                Val[i] = RationalFunc(t, i, degree, m_fKnotVector, CP);

            for (int i = 0; i < CP.Length; i++)
                ValSum += Val[i];

            for (int i = 0; i < CP.Length; ++i)
            {
                // sum effect of CV on this part of the curve
                x += Val[i] * CP[i].GetX;
                y += Val[i] * CP[i].GetY;
                z += Val[i] * CP[i].GetZ;
            }

            x /= ValSum; y /= ValSum; z /= ValSum;
        }

        /// <summary>
        /// Calculate the rational function 
        /// </summary>
        /// <param name="t"></param>
        /// <param name="k"></param>
        /// <param name="degree"></param>
        /// <param name="Knots"></param>
        /// <param name="CP"></param>
        /// <returns></returns>
        public float RationalFunc(float t, int k, int degree, float[] Knots, ControlPoints[] CP)
        {
            return CP[k].GetWeights * CoxDeBoor(t, k, degree, Knots);
        }

        /// <summary>
        /// Cox-DeBoor algorithm for calculating a spline
        /// </summary>
        /// <param name="t"></param>
        /// <param name="k"></param>
        /// <param name="degree"></param>
        /// <param name="Knots"></param>
        /// <returns></returns>
        public float CoxDeBoor(float t, int k, int degree, float[] Knots)
        {
            float B1;
            float B2;

            if (degree == 0)
            {
                if (Knots[k] <= t && t <= Knots[k + 1])
                {
                    return 1.0f;
                }
                return 0.0f;
            }

            if (Knots[k + degree] != Knots[k])
                B1 = ((t - Knots[k]) / (Knots[k + degree] - Knots[k])) * CoxDeBoor(t, k, degree - 1, Knots);
            else
                B1 = 0.0f;

            if (Knots[k + degree + 1] != Knots[k + 1])
                B2 = ((Knots[k + degree + 1] - t) / (Knots[k + degree + 1] - Knots[k + 1])) * CoxDeBoor(t, k + 1, degree - 1, Knots);
            else
                B2 = 0.0f;

            return B1 + B2;
        }

        public void MovePointstoArrays(ControlPoints[] CP, ref double[] x, ref double[] y, ref double[] z)
        {
            x = new double[CP.Length];
            y = new double[CP.Length];
            z = new double[CP.Length];

            for (int i = 0; i < CP.Length; i++)
            {
                x[i] = CP[i].GetX;
                y[i] = CP[i].GetY;
                z[i] = CP[i].GetZ;
            }
        }

        public void CreateNURBSpline(int numbsplinepts, ref double[] spline_x, ref double[] spline_y, ref double[] spline_z, ControlPoints[] CP, int degree)
        {
            spline_x = new double[numbsplinepts];
            spline_y = new double[numbsplinepts];
            spline_z = new double[numbsplinepts];

            for (int i = 0; i < numbsplinepts; ++i)
            {

                float t = m_fKnotVector[m_fKnotVector.Length - 1] * i / (float)(numbsplinepts - 1);

                if (t >= m_fKnotVector[degree])
                    GetOutpoint(t, ref spline_x[i], ref spline_y[i], ref spline_z[i], CP, degree);
            }
        }
    }
}
