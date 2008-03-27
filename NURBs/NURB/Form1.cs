using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using StochasticModeling;

namespace NURB
{
    public partial class Form1 : Form
    {
        GraphingBase m_gSpline;
        double[] m_dx;
        double[] m_dy;
        double[] m_dz;
        double[] m_dspline_x;
        double[] m_dspline_y;
        double[] m_dspline_z;
        ControlPoints[] m_CP;
        NURBGenerator nurbgen;

        //Change this to change the degree of the spline
        int degree = 6;
        //Number of points in the spline
        int numbsplinepts = 200;
        //The number of control points. Must be at least degree+1
        int numbercontrolpoints = 7;


        public Form1()
        {
            InitializeComponent();

            nurbgen = new NURBGenerator();

            //Set up our graph
            m_gSpline = new GraphingBase(string.Empty);
            m_gSpline.CreateGraph(SplineGraphControl);
            m_gSpline.SetAxisTitles(" ", " ");
            m_gSpline.Title = " ";
            m_gSpline.LegendState = false;

            NurbGenerate();
        }

        private void NurbGenerate()
        {
            //Set up the spline parameters

            nurbgen.InitControlPoints(ref m_CP, numbercontrolpoints, 1000);
            nurbgen.InitKnotVector(m_CP.Length, degree);

            //Set up our Control points
            nurbgen.MovePointstoArrays(m_CP, ref m_dx, ref m_dy, ref m_dz);

            m_gSpline.LoadfromArray("ControlPoints", m_dx, m_dy, Color.Red, ZedGraph.SymbolType.Circle, 4, false);

            //Set up the Spline
            nurbgen.CreateNURBSpline(numbsplinepts, ref m_dspline_x, ref m_dspline_y, ref m_dspline_z, m_CP, degree);

            //Add the Spline curve to the graph
            m_gSpline.LoadfromArray("SplineArray", m_dspline_x, m_dspline_y, Color.MediumBlue, ZedGraph.SymbolType.None, 4, true);
        }

        private void Resetbutton_Click(object sender, EventArgs e)
        {
            m_gSpline.Clear();
            NurbGenerate();
        }
    }
}