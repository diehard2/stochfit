namespace GIDFit
{
    partial class GIDFit
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.GIDGraph = new ZedGraph.ZedGraphControl();
            this.label1 = new System.Windows.Forms.Label();
            this.SlopeTB = new System.Windows.Forms.TextBox();
            this.Intensity6TB = new System.Windows.Forms.TextBox();
            this.Intensity5TB = new System.Windows.Forms.TextBox();
            this.Intensity4TB = new System.Windows.Forms.TextBox();
            this.Intensity3TB = new System.Windows.Forms.TextBox();
            this.Intensity1TB = new System.Windows.Forms.TextBox();
            this.Intensity2TB = new System.Windows.Forms.TextBox();
            this.ChiSquareTB = new System.Windows.Forms.TextBox();
            this.OffSetTB = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.Params = new System.Windows.Forms.GroupBox();
            this.label8 = new System.Windows.Forms.Label();
            this.Gamma6TB = new System.Windows.Forms.TextBox();
            this.Gamma5TB = new System.Windows.Forms.TextBox();
            this.Gamma4TB = new System.Windows.Forms.TextBox();
            this.Gamma3TB = new System.Windows.Forms.TextBox();
            this.Gamma1TB = new System.Windows.Forms.TextBox();
            this.Gamma2TB = new System.Windows.Forms.TextBox();
            this.label7 = new System.Windows.Forms.Label();
            this.Position6TB = new System.Windows.Forms.TextBox();
            this.Position5TB = new System.Windows.Forms.TextBox();
            this.Position4TB = new System.Windows.Forms.TextBox();
            this.Position3TB = new System.Windows.Forms.TextBox();
            this.Position1TB = new System.Windows.Forms.TextBox();
            this.Position2TB = new System.Windows.Forms.TextBox();
            this.label6 = new System.Windows.Forms.Label();
            this.Sigma6TB = new System.Windows.Forms.TextBox();
            this.Sigma5TB = new System.Windows.Forms.TextBox();
            this.Sigma4TB = new System.Windows.Forms.TextBox();
            this.Sigma3TB = new System.Windows.Forms.TextBox();
            this.Sigma1TB = new System.Windows.Forms.TextBox();
            this.Sigma2TB = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.Func2CoB = new System.Windows.Forms.ComboBox();
            this.Func3CoB = new System.Windows.Forms.ComboBox();
            this.Func4CoB = new System.Windows.Forms.ComboBox();
            this.Func5CoB = new System.Windows.Forms.ComboBox();
            this.Func6CoB = new System.Windows.Forms.ComboBox();
            this.Func1CoB = new System.Windows.Forms.ComboBox();
            this.label3 = new System.Windows.Forms.Label();
            this.FileNameTB = new System.Windows.Forms.TextBox();
            this.LoadFileBT = new System.Windows.Forms.Button();
            this.openFileDialog1 = new System.Windows.Forms.OpenFileDialog();
            this.FitBT = new System.Windows.Forms.Button();
            this.FunctionNumberTB = new System.Windows.Forms.TextBox();
            this.label9 = new System.Windows.Forms.Label();
            this.FWHM1 = new System.Windows.Forms.TextBox();
            this.FWHM4 = new System.Windows.Forms.TextBox();
            this.FWHM5 = new System.Windows.Forms.TextBox();
            this.FWHM6 = new System.Windows.Forms.TextBox();
            this.FWHM3 = new System.Windows.Forms.TextBox();
            this.FWHM2 = new System.Windows.Forms.TextBox();
            this.label10 = new System.Windows.Forms.Label();
            this.pubgraph = new System.Windows.Forms.Button();
            this.Params.SuspendLayout();
            this.SuspendLayout();
            // 
            // GIDGraph
            // 
            this.GIDGraph.Anchor = System.Windows.Forms.AnchorStyles.None;
            this.GIDGraph.EditButtons = System.Windows.Forms.MouseButtons.Left;
            this.GIDGraph.Location = new System.Drawing.Point(12, 12);
            this.GIDGraph.Name = "GIDGraph";
            this.GIDGraph.PanModifierKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Shift | System.Windows.Forms.Keys.None)));
            this.GIDGraph.ScrollGrace = 0;
            this.GIDGraph.ScrollMaxX = 0;
            this.GIDGraph.ScrollMaxY = 0;
            this.GIDGraph.ScrollMaxY2 = 0;
            this.GIDGraph.ScrollMinX = 0;
            this.GIDGraph.ScrollMinY = 0;
            this.GIDGraph.ScrollMinY2 = 0;
            this.GIDGraph.Size = new System.Drawing.Size(559, 340);
            this.GIDGraph.TabIndex = 10;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(593, 104);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(34, 13);
            this.label1.TabIndex = 11;
            this.label1.Text = "Slope";
            // 
            // SlopeTB
            // 
            this.SlopeTB.Enabled = false;
            this.SlopeTB.Location = new System.Drawing.Point(596, 120);
            this.SlopeTB.Name = "SlopeTB";
            this.SlopeTB.Size = new System.Drawing.Size(100, 20);
            this.SlopeTB.TabIndex = 0;
            this.SlopeTB.Text = "0";
            this.SlopeTB.Validated += new System.EventHandler(this.SlopeTB_Validated);
            this.SlopeTB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Intensity6TB
            // 
            this.Intensity6TB.Location = new System.Drawing.Point(26, 249);
            this.Intensity6TB.Name = "Intensity6TB";
            this.Intensity6TB.Size = new System.Drawing.Size(100, 20);
            this.Intensity6TB.TabIndex = 25;
            this.Intensity6TB.Text = "10000";
            this.Intensity6TB.Validated += new System.EventHandler(this.Intensity6TB_Validated);
            this.Intensity6TB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Intensity5TB
            // 
            this.Intensity5TB.Location = new System.Drawing.Point(26, 209);
            this.Intensity5TB.Name = "Intensity5TB";
            this.Intensity5TB.Size = new System.Drawing.Size(100, 20);
            this.Intensity5TB.TabIndex = 20;
            this.Intensity5TB.Text = "10000";
            this.Intensity5TB.Validated += new System.EventHandler(this.Intensity5TB_Validated);
            this.Intensity5TB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Intensity4TB
            // 
            this.Intensity4TB.Location = new System.Drawing.Point(26, 169);
            this.Intensity4TB.Name = "Intensity4TB";
            this.Intensity4TB.Size = new System.Drawing.Size(100, 20);
            this.Intensity4TB.TabIndex = 15;
            this.Intensity4TB.Text = "10000";
            this.Intensity4TB.Validated += new System.EventHandler(this.Intensity4TB_Validated);
            this.Intensity4TB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Intensity3TB
            // 
            this.Intensity3TB.Location = new System.Drawing.Point(26, 129);
            this.Intensity3TB.Name = "Intensity3TB";
            this.Intensity3TB.Size = new System.Drawing.Size(100, 20);
            this.Intensity3TB.TabIndex = 10;
            this.Intensity3TB.Text = "10000";
            this.Intensity3TB.Validated += new System.EventHandler(this.Intensity3TB_Validated);
            this.Intensity3TB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Intensity1TB
            // 
            this.Intensity1TB.Location = new System.Drawing.Point(26, 49);
            this.Intensity1TB.Name = "Intensity1TB";
            this.Intensity1TB.Size = new System.Drawing.Size(100, 20);
            this.Intensity1TB.TabIndex = 0;
            this.Intensity1TB.Text = "10000";
            this.Intensity1TB.Validated += new System.EventHandler(this.Intensity1TB_Validated);
            this.Intensity1TB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Intensity2TB
            // 
            this.Intensity2TB.Location = new System.Drawing.Point(26, 89);
            this.Intensity2TB.Name = "Intensity2TB";
            this.Intensity2TB.Size = new System.Drawing.Size(100, 20);
            this.Intensity2TB.TabIndex = 5;
            this.Intensity2TB.Text = "5000";
            this.Intensity2TB.Validated += new System.EventHandler(this.Intensity2TB_Validated);
            this.Intensity2TB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // ChiSquareTB
            // 
            this.ChiSquareTB.Location = new System.Drawing.Point(596, 275);
            this.ChiSquareTB.Name = "ChiSquareTB";
            this.ChiSquareTB.ReadOnly = true;
            this.ChiSquareTB.Size = new System.Drawing.Size(100, 20);
            this.ChiSquareTB.TabIndex = 22;
            // 
            // OffSetTB
            // 
            this.OffSetTB.Enabled = false;
            this.OffSetTB.Location = new System.Drawing.Point(596, 191);
            this.OffSetTB.Name = "OffSetTB";
            this.OffSetTB.Size = new System.Drawing.Size(100, 20);
            this.OffSetTB.TabIndex = 2;
            this.OffSetTB.Text = "25000";
            this.OffSetTB.Validated += new System.EventHandler(this.OffSetTB_Validated);
            this.OffSetTB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(593, 175);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(35, 13);
            this.label2.TabIndex = 24;
            this.label2.Text = "Offset";
            // 
            // Params
            // 
            this.Params.Controls.Add(this.label8);
            this.Params.Controls.Add(this.Gamma6TB);
            this.Params.Controls.Add(this.Gamma5TB);
            this.Params.Controls.Add(this.Gamma4TB);
            this.Params.Controls.Add(this.Gamma3TB);
            this.Params.Controls.Add(this.Gamma1TB);
            this.Params.Controls.Add(this.Gamma2TB);
            this.Params.Controls.Add(this.label7);
            this.Params.Controls.Add(this.Position6TB);
            this.Params.Controls.Add(this.Position5TB);
            this.Params.Controls.Add(this.Position4TB);
            this.Params.Controls.Add(this.Position3TB);
            this.Params.Controls.Add(this.Position1TB);
            this.Params.Controls.Add(this.Position2TB);
            this.Params.Controls.Add(this.label6);
            this.Params.Controls.Add(this.Sigma6TB);
            this.Params.Controls.Add(this.Sigma5TB);
            this.Params.Controls.Add(this.Sigma4TB);
            this.Params.Controls.Add(this.Sigma3TB);
            this.Params.Controls.Add(this.Sigma1TB);
            this.Params.Controls.Add(this.Sigma2TB);
            this.Params.Controls.Add(this.label5);
            this.Params.Controls.Add(this.label4);
            this.Params.Controls.Add(this.Func2CoB);
            this.Params.Controls.Add(this.Func3CoB);
            this.Params.Controls.Add(this.Func4CoB);
            this.Params.Controls.Add(this.Func5CoB);
            this.Params.Controls.Add(this.Intensity6TB);
            this.Params.Controls.Add(this.Intensity5TB);
            this.Params.Controls.Add(this.Intensity4TB);
            this.Params.Controls.Add(this.Intensity3TB);
            this.Params.Controls.Add(this.Intensity1TB);
            this.Params.Controls.Add(this.Intensity2TB);
            this.Params.Controls.Add(this.Func6CoB);
            this.Params.Controls.Add(this.Func1CoB);
            this.Params.Location = new System.Drawing.Point(17, 361);
            this.Params.Name = "Params";
            this.Params.Size = new System.Drawing.Size(724, 287);
            this.Params.TabIndex = 25;
            this.Params.TabStop = false;
            this.Params.Text = "Parameters";
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(451, 16);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(43, 13);
            this.label8.TabIndex = 49;
            this.label8.Text = "Gamma";
            // 
            // Gamma6TB
            // 
            this.Gamma6TB.Location = new System.Drawing.Point(454, 249);
            this.Gamma6TB.Name = "Gamma6TB";
            this.Gamma6TB.Size = new System.Drawing.Size(100, 20);
            this.Gamma6TB.TabIndex = 28;
            this.Gamma6TB.Text = "0";
            this.Gamma6TB.Validated += new System.EventHandler(this.Gamma6TB_Validated);
            this.Gamma6TB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Gamma5TB
            // 
            this.Gamma5TB.Location = new System.Drawing.Point(454, 209);
            this.Gamma5TB.Name = "Gamma5TB";
            this.Gamma5TB.Size = new System.Drawing.Size(100, 20);
            this.Gamma5TB.TabIndex = 23;
            this.Gamma5TB.Text = "0";
            this.Gamma5TB.Validated += new System.EventHandler(this.Gamma5TB_Validated);
            this.Gamma5TB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Gamma4TB
            // 
            this.Gamma4TB.Location = new System.Drawing.Point(454, 169);
            this.Gamma4TB.Name = "Gamma4TB";
            this.Gamma4TB.Size = new System.Drawing.Size(100, 20);
            this.Gamma4TB.TabIndex = 18;
            this.Gamma4TB.Text = "0";
            this.Gamma4TB.Validated += new System.EventHandler(this.Gamma4TB_Validated);
            this.Gamma4TB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Gamma3TB
            // 
            this.Gamma3TB.Location = new System.Drawing.Point(454, 129);
            this.Gamma3TB.Name = "Gamma3TB";
            this.Gamma3TB.Size = new System.Drawing.Size(100, 20);
            this.Gamma3TB.TabIndex = 13;
            this.Gamma3TB.Text = "0";
            this.Gamma3TB.Validated += new System.EventHandler(this.Gamma3TB_Validated);
            this.Gamma3TB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Gamma1TB
            // 
            this.Gamma1TB.Location = new System.Drawing.Point(454, 49);
            this.Gamma1TB.Name = "Gamma1TB";
            this.Gamma1TB.Size = new System.Drawing.Size(100, 20);
            this.Gamma1TB.TabIndex = 3;
            this.Gamma1TB.Text = ".03";
            this.Gamma1TB.Validated += new System.EventHandler(this.Gamma1TB_Validated);
            this.Gamma1TB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Gamma2TB
            // 
            this.Gamma2TB.Location = new System.Drawing.Point(454, 89);
            this.Gamma2TB.Name = "Gamma2TB";
            this.Gamma2TB.Size = new System.Drawing.Size(100, 20);
            this.Gamma2TB.TabIndex = 8;
            this.Gamma2TB.Text = "0";
            this.Gamma2TB.Validated += new System.EventHandler(this.Gamma2TB_Validated);
            this.Gamma2TB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(311, 15);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(44, 13);
            this.label7.TabIndex = 42;
            this.label7.Text = "Position";
            // 
            // Position6TB
            // 
            this.Position6TB.Location = new System.Drawing.Point(314, 248);
            this.Position6TB.Name = "Position6TB";
            this.Position6TB.Size = new System.Drawing.Size(100, 20);
            this.Position6TB.TabIndex = 27;
            this.Position6TB.Text = "1.45";
            this.Position6TB.Validated += new System.EventHandler(this.Position6TB_Validated);
            this.Position6TB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Position5TB
            // 
            this.Position5TB.Location = new System.Drawing.Point(314, 208);
            this.Position5TB.Name = "Position5TB";
            this.Position5TB.Size = new System.Drawing.Size(100, 20);
            this.Position5TB.TabIndex = 22;
            this.Position5TB.Text = "1.45";
            this.Position5TB.Validated += new System.EventHandler(this.Position5TB_Validated);
            this.Position5TB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Position4TB
            // 
            this.Position4TB.Location = new System.Drawing.Point(314, 168);
            this.Position4TB.Name = "Position4TB";
            this.Position4TB.Size = new System.Drawing.Size(100, 20);
            this.Position4TB.TabIndex = 17;
            this.Position4TB.Text = "1.45";
            this.Position4TB.Validated += new System.EventHandler(this.Position4TB_Validated);
            this.Position4TB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Position3TB
            // 
            this.Position3TB.Location = new System.Drawing.Point(314, 128);
            this.Position3TB.Name = "Position3TB";
            this.Position3TB.Size = new System.Drawing.Size(100, 20);
            this.Position3TB.TabIndex = 12;
            this.Position3TB.Text = "1.45";
            this.Position3TB.Validated += new System.EventHandler(this.Position3TB_Validated);
            this.Position3TB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Position1TB
            // 
            this.Position1TB.Location = new System.Drawing.Point(314, 48);
            this.Position1TB.Name = "Position1TB";
            this.Position1TB.Size = new System.Drawing.Size(100, 20);
            this.Position1TB.TabIndex = 2;
            this.Position1TB.Text = "1.45";
            this.Position1TB.Validated += new System.EventHandler(this.Position1TB_Validated);
            this.Position1TB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Position2TB
            // 
            this.Position2TB.Location = new System.Drawing.Point(314, 88);
            this.Position2TB.Name = "Position2TB";
            this.Position2TB.Size = new System.Drawing.Size(100, 20);
            this.Position2TB.TabIndex = 7;
            this.Position2TB.Text = "1.42";
            this.Position2TB.Validated += new System.EventHandler(this.Position2TB_Validated);
            this.Position2TB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(166, 15);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(36, 13);
            this.label6.TabIndex = 35;
            this.label6.Text = "Sigma";
            // 
            // Sigma6TB
            // 
            this.Sigma6TB.Location = new System.Drawing.Point(169, 248);
            this.Sigma6TB.Name = "Sigma6TB";
            this.Sigma6TB.Size = new System.Drawing.Size(100, 20);
            this.Sigma6TB.TabIndex = 26;
            this.Sigma6TB.Text = "0.03";
            this.Sigma6TB.Validated += new System.EventHandler(this.Sigma6TB_Validated);
            this.Sigma6TB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Sigma5TB
            // 
            this.Sigma5TB.Location = new System.Drawing.Point(169, 208);
            this.Sigma5TB.Name = "Sigma5TB";
            this.Sigma5TB.Size = new System.Drawing.Size(100, 20);
            this.Sigma5TB.TabIndex = 21;
            this.Sigma5TB.Text = "0.03";
            this.Sigma5TB.Validated += new System.EventHandler(this.Sigma5TB_Validated);
            this.Sigma5TB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Sigma4TB
            // 
            this.Sigma4TB.Location = new System.Drawing.Point(169, 168);
            this.Sigma4TB.Name = "Sigma4TB";
            this.Sigma4TB.Size = new System.Drawing.Size(100, 20);
            this.Sigma4TB.TabIndex = 16;
            this.Sigma4TB.Text = "0.03";
            this.Sigma4TB.Validated += new System.EventHandler(this.Sigma4TB_Validated);
            this.Sigma4TB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Sigma3TB
            // 
            this.Sigma3TB.Location = new System.Drawing.Point(169, 128);
            this.Sigma3TB.Name = "Sigma3TB";
            this.Sigma3TB.Size = new System.Drawing.Size(100, 20);
            this.Sigma3TB.TabIndex = 11;
            this.Sigma3TB.Text = "0.03";
            this.Sigma3TB.Validated += new System.EventHandler(this.Sigma3TB_Validated);
            this.Sigma3TB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Sigma1TB
            // 
            this.Sigma1TB.Location = new System.Drawing.Point(169, 48);
            this.Sigma1TB.Name = "Sigma1TB";
            this.Sigma1TB.Size = new System.Drawing.Size(100, 20);
            this.Sigma1TB.TabIndex = 1;
            this.Sigma1TB.Text = "0.03";
            this.Sigma1TB.Validated += new System.EventHandler(this.Sigma1TB_Validated);
            this.Sigma1TB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Sigma2TB
            // 
            this.Sigma2TB.Location = new System.Drawing.Point(169, 88);
            this.Sigma2TB.Name = "Sigma2TB";
            this.Sigma2TB.Size = new System.Drawing.Size(100, 20);
            this.Sigma2TB.TabIndex = 6;
            this.Sigma2TB.Text = "0.03";
            this.Sigma2TB.Validated += new System.EventHandler(this.Sigma2TB_Validated);
            this.Sigma2TB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(23, 16);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(46, 13);
            this.label5.TabIndex = 28;
            this.label5.Text = "Intensity";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(584, 16);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(48, 13);
            this.label4.TabIndex = 27;
            this.label4.Text = "Function";
            // 
            // Func2CoB
            // 
            this.Func2CoB.DisplayMember = "0";
            this.Func2CoB.FormattingEnabled = true;
            this.Func2CoB.Items.AddRange(new object[] {
            "Gaussian",
            "Lorentzian",
            "Pearson VII"});
            this.Func2CoB.Location = new System.Drawing.Point(579, 88);
            this.Func2CoB.Name = "Func2CoB";
            this.Func2CoB.Size = new System.Drawing.Size(121, 21);
            this.Func2CoB.TabIndex = 9;
            this.Func2CoB.SelectedIndexChanged += new System.EventHandler(this.Func2CoB_SelectedIndexChanged);
            // 
            // Func3CoB
            // 
            this.Func3CoB.DisplayMember = "0";
            this.Func3CoB.FormattingEnabled = true;
            this.Func3CoB.Items.AddRange(new object[] {
            "Gaussian",
            "Lorentzian",
            "Pearson VII"});
            this.Func3CoB.Location = new System.Drawing.Point(579, 128);
            this.Func3CoB.Name = "Func3CoB";
            this.Func3CoB.Size = new System.Drawing.Size(121, 21);
            this.Func3CoB.TabIndex = 14;
            this.Func3CoB.SelectedIndexChanged += new System.EventHandler(this.Func3CoB_SelectedIndexChanged);
            // 
            // Func4CoB
            // 
            this.Func4CoB.DisplayMember = "0";
            this.Func4CoB.FormattingEnabled = true;
            this.Func4CoB.Items.AddRange(new object[] {
            "Gaussian",
            "Lorentzian",
            "Pearson VII"});
            this.Func4CoB.Location = new System.Drawing.Point(579, 168);
            this.Func4CoB.Name = "Func4CoB";
            this.Func4CoB.Size = new System.Drawing.Size(121, 21);
            this.Func4CoB.TabIndex = 19;
            this.Func4CoB.SelectionChangeCommitted += new System.EventHandler(this.Func4CoB_SelectionChangeCommitted);
            // 
            // Func5CoB
            // 
            this.Func5CoB.DisplayMember = "0";
            this.Func5CoB.FormattingEnabled = true;
            this.Func5CoB.Items.AddRange(new object[] {
            "Gaussian",
            "Lorentzian",
            "Pearson VII"});
            this.Func5CoB.Location = new System.Drawing.Point(579, 208);
            this.Func5CoB.Name = "Func5CoB";
            this.Func5CoB.Size = new System.Drawing.Size(121, 21);
            this.Func5CoB.TabIndex = 24;
            this.Func5CoB.SelectedIndexChanged += new System.EventHandler(this.Func5CoB_SelectedIndexChanged);
            // 
            // Func6CoB
            // 
            this.Func6CoB.DisplayMember = "0";
            this.Func6CoB.FormattingEnabled = true;
            this.Func6CoB.Items.AddRange(new object[] {
            "Gaussian",
            "Lorentzian",
            "Pearson VII"});
            this.Func6CoB.Location = new System.Drawing.Point(579, 248);
            this.Func6CoB.Name = "Func6CoB";
            this.Func6CoB.Size = new System.Drawing.Size(121, 21);
            this.Func6CoB.TabIndex = 29;
            this.Func6CoB.SelectedIndexChanged += new System.EventHandler(this.Func6CoB_SelectedIndexChanged);
            // 
            // Func1CoB
            // 
            this.Func1CoB.DisplayMember = "0";
            this.Func1CoB.FormattingEnabled = true;
            this.Func1CoB.Items.AddRange(new object[] {
            "Gaussian",
            "Lorentzian",
            "Pearson VII"});
            this.Func1CoB.Location = new System.Drawing.Point(579, 48);
            this.Func1CoB.Name = "Func1CoB";
            this.Func1CoB.Size = new System.Drawing.Size(121, 21);
            this.Func1CoB.TabIndex = 4;
            this.Func1CoB.SelectedIndexChanged += new System.EventHandler(this.Func1CoB_SelectedIndexChanged);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(592, 259);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(59, 13);
            this.label3.TabIndex = 26;
            this.label3.Text = "Chi Square";
            // 
            // FileNameTB
            // 
            this.FileNameTB.Location = new System.Drawing.Point(596, 16);
            this.FileNameTB.Name = "FileNameTB";
            this.FileNameTB.ReadOnly = true;
            this.FileNameTB.Size = new System.Drawing.Size(255, 20);
            this.FileNameTB.TabIndex = 27;
            // 
            // LoadFileBT
            // 
            this.LoadFileBT.Location = new System.Drawing.Point(595, 46);
            this.LoadFileBT.Name = "LoadFileBT";
            this.LoadFileBT.Size = new System.Drawing.Size(76, 23);
            this.LoadFileBT.TabIndex = 0;
            this.LoadFileBT.Text = "Load File";
            this.LoadFileBT.UseVisualStyleBackColor = true;
            this.LoadFileBT.Click += new System.EventHandler(this.LoadFileBT_Click);
            // 
            // openFileDialog1
            // 
            this.openFileDialog1.FileName = "openFileDialog1";
            // 
            // FitBT
            // 
            this.FitBT.Enabled = false;
            this.FitBT.Location = new System.Drawing.Point(776, 46);
            this.FitBT.Name = "FitBT";
            this.FitBT.Size = new System.Drawing.Size(75, 23);
            this.FitBT.TabIndex = 2;
            this.FitBT.Text = "Fit";
            this.FitBT.UseVisualStyleBackColor = true;
            this.FitBT.Click += new System.EventHandler(this.FitBT_Click);
            // 
            // FunctionNumberTB
            // 
            this.FunctionNumberTB.Enabled = false;
            this.FunctionNumberTB.Location = new System.Drawing.Point(751, 120);
            this.FunctionNumberTB.Name = "FunctionNumberTB";
            this.FunctionNumberTB.Size = new System.Drawing.Size(100, 20);
            this.FunctionNumberTB.TabIndex = 1;
            this.FunctionNumberTB.Text = "2";
            this.FunctionNumberTB.Validated += new System.EventHandler(this.FunctionNumberTB_TextChanged);
            this.FunctionNumberTB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateIntegerInput);
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(748, 104);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(88, 13);
            this.label9.TabIndex = 30;
            this.label9.Text = "Function Number";
            // 
            // FWHM1
            // 
            this.FWHM1.Location = new System.Drawing.Point(762, 409);
            this.FWHM1.Name = "FWHM1";
            this.FWHM1.ReadOnly = true;
            this.FWHM1.Size = new System.Drawing.Size(100, 20);
            this.FWHM1.TabIndex = 31;
            this.FWHM1.TabStop = false;
            // 
            // FWHM4
            // 
            this.FWHM4.Location = new System.Drawing.Point(762, 530);
            this.FWHM4.Name = "FWHM4";
            this.FWHM4.ReadOnly = true;
            this.FWHM4.Size = new System.Drawing.Size(100, 20);
            this.FWHM4.TabIndex = 32;
            this.FWHM4.TabStop = false;
            // 
            // FWHM5
            // 
            this.FWHM5.Location = new System.Drawing.Point(762, 569);
            this.FWHM5.Name = "FWHM5";
            this.FWHM5.ReadOnly = true;
            this.FWHM5.Size = new System.Drawing.Size(100, 20);
            this.FWHM5.TabIndex = 33;
            this.FWHM5.TabStop = false;
            // 
            // FWHM6
            // 
            this.FWHM6.Location = new System.Drawing.Point(762, 610);
            this.FWHM6.Name = "FWHM6";
            this.FWHM6.ReadOnly = true;
            this.FWHM6.Size = new System.Drawing.Size(100, 20);
            this.FWHM6.TabIndex = 34;
            this.FWHM6.TabStop = false;
            // 
            // FWHM3
            // 
            this.FWHM3.Location = new System.Drawing.Point(762, 490);
            this.FWHM3.Name = "FWHM3";
            this.FWHM3.ReadOnly = true;
            this.FWHM3.Size = new System.Drawing.Size(100, 20);
            this.FWHM3.TabIndex = 35;
            this.FWHM3.TabStop = false;
            // 
            // FWHM2
            // 
            this.FWHM2.Location = new System.Drawing.Point(762, 449);
            this.FWHM2.Name = "FWHM2";
            this.FWHM2.ReadOnly = true;
            this.FWHM2.Size = new System.Drawing.Size(100, 20);
            this.FWHM2.TabIndex = 36;
            this.FWHM2.TabStop = false;
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Location = new System.Drawing.Point(759, 376);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(41, 13);
            this.label10.TabIndex = 50;
            this.label10.Text = "FWHM";
            // 
            // pubgraph
            // 
            this.pubgraph.Location = new System.Drawing.Point(740, 189);
            this.pubgraph.Name = "pubgraph";
            this.pubgraph.Size = new System.Drawing.Size(122, 22);
            this.pubgraph.TabIndex = 51;
            this.pubgraph.Text = "Publication graph";
            this.pubgraph.UseVisualStyleBackColor = true;
            this.pubgraph.Click += new System.EventHandler(this.pubgraph_Click);
            // 
            // GIDFit
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(874, 660);
            this.Controls.Add(this.pubgraph);
            this.Controls.Add(this.label10);
            this.Controls.Add(this.FWHM2);
            this.Controls.Add(this.FWHM3);
            this.Controls.Add(this.FWHM6);
            this.Controls.Add(this.FWHM5);
            this.Controls.Add(this.FWHM4);
            this.Controls.Add(this.FWHM1);
            this.Controls.Add(this.FunctionNumberTB);
            this.Controls.Add(this.label9);
            this.Controls.Add(this.FitBT);
            this.Controls.Add(this.LoadFileBT);
            this.Controls.Add(this.FileNameTB);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.Params);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.OffSetTB);
            this.Controls.Add(this.ChiSquareTB);
            this.Controls.Add(this.SlopeTB);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.GIDGraph);
            this.Name = "GIDFit";
            this.Text = "GIDFit";
            this.Load += new System.EventHandler(this.GIDFit_Load);
            this.Params.ResumeLayout(false);
            this.Params.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private ZedGraph.ZedGraphControl GIDGraph;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox SlopeTB;
        private System.Windows.Forms.TextBox Intensity6TB;
        private System.Windows.Forms.TextBox Intensity5TB;
        private System.Windows.Forms.TextBox Intensity4TB;
        private System.Windows.Forms.TextBox Intensity3TB;
        private System.Windows.Forms.TextBox Intensity1TB;
        private System.Windows.Forms.TextBox Intensity2TB;
        private System.Windows.Forms.TextBox ChiSquareTB;
        private System.Windows.Forms.TextBox OffSetTB;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.GroupBox Params;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.TextBox Sigma6TB;
        private System.Windows.Forms.TextBox Sigma5TB;
        private System.Windows.Forms.TextBox Sigma4TB;
        private System.Windows.Forms.TextBox Sigma3TB;
        private System.Windows.Forms.TextBox Sigma1TB;
        private System.Windows.Forms.TextBox Sigma2TB;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.ComboBox Func2CoB;
        private System.Windows.Forms.ComboBox Func3CoB;
        private System.Windows.Forms.ComboBox Func4CoB;
        private System.Windows.Forms.ComboBox Func5CoB;
        private System.Windows.Forms.ComboBox Func6CoB;
        private System.Windows.Forms.ComboBox Func1CoB;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.TextBox Gamma6TB;
        private System.Windows.Forms.TextBox Gamma5TB;
        private System.Windows.Forms.TextBox Gamma4TB;
        private System.Windows.Forms.TextBox Gamma3TB;
        private System.Windows.Forms.TextBox Gamma1TB;
        private System.Windows.Forms.TextBox Gamma2TB;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.TextBox Position6TB;
        private System.Windows.Forms.TextBox Position5TB;
        private System.Windows.Forms.TextBox Position4TB;
        private System.Windows.Forms.TextBox Position3TB;
        private System.Windows.Forms.TextBox Position1TB;
        private System.Windows.Forms.TextBox Position2TB;
        private System.Windows.Forms.TextBox FileNameTB;
        private System.Windows.Forms.Button LoadFileBT;
        private System.Windows.Forms.OpenFileDialog openFileDialog1;
        private System.Windows.Forms.Button FitBT;
        private System.Windows.Forms.TextBox FunctionNumberTB;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.TextBox FWHM1;
        private System.Windows.Forms.TextBox FWHM4;
        private System.Windows.Forms.TextBox FWHM5;
        private System.Windows.Forms.TextBox FWHM6;
        private System.Windows.Forms.TextBox FWHM3;
        private System.Windows.Forms.TextBox FWHM2;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.Button pubgraph;

    }
}

