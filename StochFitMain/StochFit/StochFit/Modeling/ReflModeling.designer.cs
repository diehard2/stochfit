namespace StochasticModeling
{
    partial class Reflmodeling
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Reflmodeling));
            this.RhoGraph = new ZedGraph.ZedGraphControl();
            this.LevenbergFit = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.BoxCount = new System.Windows.Forms.TextBox();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.Rho6 = new System.Windows.Forms.TextBox();
            this.LLength6 = new System.Windows.Forms.TextBox();
            this.Sigma6 = new System.Windows.Forms.TextBox();
            this.LLength5 = new System.Windows.Forms.TextBox();
            this.Sigma4 = new System.Windows.Forms.TextBox();
            this.LLength4 = new System.Windows.Forms.TextBox();
            this.Rho5 = new System.Windows.Forms.TextBox();
            this.Rho4 = new System.Windows.Forms.TextBox();
            this.Sigma5 = new System.Windows.Forms.TextBox();
            this.Sigma3 = new System.Windows.Forms.TextBox();
            this.LLength3 = new System.Windows.Forms.TextBox();
            this.Rho3 = new System.Windows.Forms.TextBox();
            this.Sigma2 = new System.Windows.Forms.TextBox();
            this.LLength2 = new System.Windows.Forms.TextBox();
            this.Rho2 = new System.Windows.Forms.TextBox();
            this.Sigma1 = new System.Windows.Forms.TextBox();
            this.LLength1 = new System.Windows.Forms.TextBox();
            this.Rho1 = new System.Windows.Forms.TextBox();
            this.label11 = new System.Windows.Forms.Label();
            this.label9 = new System.Windows.Forms.Label();
            this.RhoLabel = new System.Windows.Forms.Label();
            this.label8 = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.Holdsigma = new System.Windows.Forms.CheckBox();
            this.label12 = new System.Windows.Forms.Label();
            this.SubphaseSLD = new System.Windows.Forms.TextBox();
            this.label13 = new System.Windows.Forms.Label();
            this.UndoFit = new System.Windows.Forms.Button();
            this.label15 = new System.Windows.Forms.Label();
            this.chisquaretb = new System.Windows.Forms.TextBox();
            this.SubRough = new System.Windows.Forms.TextBox();
            this.label14 = new System.Windows.Forms.Label();
            this.CritOffset = new System.Windows.Forms.TextBox();
            this.label17 = new System.Windows.Forms.Label();
            this.Rightoffset = new System.Windows.Forms.TextBox();
            this.button1 = new System.Windows.Forms.Button();
            this.button2 = new System.Windows.Forms.Button();
            this.loadingCircle1 = new MRG.Controls.UI.LoadingCircle();
            this.label16 = new System.Windows.Forms.Label();
            this.SupSLDTB = new System.Windows.Forms.TextBox();
            this.label18 = new System.Windows.Forms.Label();
            this.QSpreadTB = new System.Windows.Forms.TextBox();
            this.DBFCB = new System.Windows.Forms.CheckBox();
            this.NormCorrectCB = new System.Windows.Forms.CheckBox();
            this.label19 = new System.Windows.Forms.Label();
            this.NormCorrectTB = new System.Windows.Forms.TextBox();
            this.label20 = new System.Windows.Forms.Label();
            this.WavelengthTB = new System.Windows.Forms.TextBox();
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.loadingCircle2 = new MRG.Controls.UI.LoadingCircle();
            this.EDzedGraphControl1 = new ZedGraph.ZedGraphControl();
            this.controlbox = new System.Windows.Forms.GroupBox();
            this.groupBox1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.tabControl1.SuspendLayout();
            this.tabPage1.SuspendLayout();
            this.tabPage2.SuspendLayout();
            this.controlbox.SuspendLayout();
            this.SuspendLayout();
            // 
            // RhoGraph
            // 
            this.RhoGraph.Anchor = System.Windows.Forms.AnchorStyles.None;
            this.RhoGraph.AutoSize = true;
            this.RhoGraph.EditButtons = System.Windows.Forms.MouseButtons.Left;
            this.RhoGraph.Location = new System.Drawing.Point(3, 3);
            this.RhoGraph.Name = "RhoGraph";
            this.RhoGraph.PanModifierKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Shift | System.Windows.Forms.Keys.None)));
            this.RhoGraph.ScrollGrace = 0;
            this.RhoGraph.ScrollMaxX = 0;
            this.RhoGraph.ScrollMaxY = 0;
            this.RhoGraph.ScrollMaxY2 = 0;
            this.RhoGraph.ScrollMinX = 0;
            this.RhoGraph.ScrollMinY = 0;
            this.RhoGraph.ScrollMinY2 = 0;
            this.RhoGraph.Size = new System.Drawing.Size(401, 340);
            this.RhoGraph.TabIndex = 2;
            // 
            // LevenbergFit
            // 
            this.LevenbergFit.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.LevenbergFit.Location = new System.Drawing.Point(467, 366);
            this.LevenbergFit.Name = "LevenbergFit";
            this.LevenbergFit.Size = new System.Drawing.Size(108, 24);
            this.LevenbergFit.TabIndex = 2;
            this.LevenbergFit.Text = "Fit";
            this.LevenbergFit.UseVisualStyleBackColor = true;
            this.LevenbergFit.Click += new System.EventHandler(this.LevenbergFit_Click);
            // 
            // label1
            // 
            this.label1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.label1.AutoSize = true;
            this.label1.Enabled = false;
            this.label1.Location = new System.Drawing.Point(21, 14);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(88, 13);
            this.label1.TabIndex = 7;
            this.label1.Text = "Number of Boxes";
            // 
            // BoxCount
            // 
            this.BoxCount.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.BoxCount.Location = new System.Drawing.Point(24, 29);
            this.BoxCount.Name = "BoxCount";
            this.BoxCount.Size = new System.Drawing.Size(26, 20);
            this.BoxCount.TabIndex = 0;
            this.BoxCount.Text = "2";
            this.BoxCount.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.BoxCount.Validated += new System.EventHandler(this.Variable_Changed);
            this.BoxCount.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateIntegerInput);
            // 
            // groupBox1
            // 
            this.groupBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.groupBox1.Controls.Add(this.Rho6);
            this.groupBox1.Controls.Add(this.LLength6);
            this.groupBox1.Controls.Add(this.Sigma6);
            this.groupBox1.Controls.Add(this.LLength5);
            this.groupBox1.Controls.Add(this.Sigma4);
            this.groupBox1.Controls.Add(this.LLength4);
            this.groupBox1.Controls.Add(this.Rho5);
            this.groupBox1.Controls.Add(this.Rho4);
            this.groupBox1.Controls.Add(this.Sigma5);
            this.groupBox1.Controls.Add(this.Sigma3);
            this.groupBox1.Controls.Add(this.LLength3);
            this.groupBox1.Controls.Add(this.Rho3);
            this.groupBox1.Controls.Add(this.Sigma2);
            this.groupBox1.Controls.Add(this.LLength2);
            this.groupBox1.Controls.Add(this.Rho2);
            this.groupBox1.Controls.Add(this.Sigma1);
            this.groupBox1.Controls.Add(this.LLength1);
            this.groupBox1.Controls.Add(this.Rho1);
            this.groupBox1.Controls.Add(this.label11);
            this.groupBox1.Controls.Add(this.label9);
            this.groupBox1.Controls.Add(this.RhoLabel);
            this.groupBox1.Controls.Add(this.label8);
            this.groupBox1.Controls.Add(this.label7);
            this.groupBox1.Controls.Add(this.label6);
            this.groupBox1.Controls.Add(this.label5);
            this.groupBox1.Controls.Add(this.label4);
            this.groupBox1.Controls.Add(this.label3);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Location = new System.Drawing.Point(12, 403);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(392, 203);
            this.groupBox1.TabIndex = 1;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Layer Parameters";
            // 
            // Rho6
            // 
            this.Rho6.Location = new System.Drawing.Point(88, 171);
            this.Rho6.Name = "Rho6";
            this.Rho6.Size = new System.Drawing.Size(76, 20);
            this.Rho6.TabIndex = 15;
            this.Rho6.Text = "1";
            this.Rho6.Validated += new System.EventHandler(this.Variable_Changed);
            this.Rho6.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // LLength6
            // 
            this.LLength6.Location = new System.Drawing.Point(190, 171);
            this.LLength6.Name = "LLength6";
            this.LLength6.Size = new System.Drawing.Size(76, 20);
            this.LLength6.TabIndex = 16;
            this.LLength6.Text = "10";
            this.LLength6.Validated += new System.EventHandler(this.Variable_Changed);
            this.LLength6.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Sigma6
            // 
            this.Sigma6.Location = new System.Drawing.Point(286, 171);
            this.Sigma6.Name = "Sigma6";
            this.Sigma6.Size = new System.Drawing.Size(76, 20);
            this.Sigma6.TabIndex = 17;
            this.Sigma6.Text = "3";
            this.Sigma6.Validated += new System.EventHandler(this.Variable_Changed);
            this.Sigma6.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // LLength5
            // 
            this.LLength5.Location = new System.Drawing.Point(190, 146);
            this.LLength5.Name = "LLength5";
            this.LLength5.Size = new System.Drawing.Size(76, 20);
            this.LLength5.TabIndex = 13;
            this.LLength5.Text = "10";
            this.LLength5.Validated += new System.EventHandler(this.Variable_Changed);
            this.LLength5.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Sigma4
            // 
            this.Sigma4.Location = new System.Drawing.Point(286, 121);
            this.Sigma4.Name = "Sigma4";
            this.Sigma4.Size = new System.Drawing.Size(76, 20);
            this.Sigma4.TabIndex = 11;
            this.Sigma4.Text = "3";
            this.Sigma4.Validated += new System.EventHandler(this.Variable_Changed);
            this.Sigma4.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // LLength4
            // 
            this.LLength4.Location = new System.Drawing.Point(190, 121);
            this.LLength4.Name = "LLength4";
            this.LLength4.Size = new System.Drawing.Size(76, 20);
            this.LLength4.TabIndex = 10;
            this.LLength4.Text = "10";
            this.LLength4.Validated += new System.EventHandler(this.Variable_Changed);
            this.LLength4.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Rho5
            // 
            this.Rho5.Location = new System.Drawing.Point(88, 146);
            this.Rho5.Name = "Rho5";
            this.Rho5.Size = new System.Drawing.Size(76, 20);
            this.Rho5.TabIndex = 12;
            this.Rho5.Text = "1";
            this.Rho5.Validated += new System.EventHandler(this.Variable_Changed);
            this.Rho5.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Rho4
            // 
            this.Rho4.Location = new System.Drawing.Point(88, 121);
            this.Rho4.Name = "Rho4";
            this.Rho4.Size = new System.Drawing.Size(76, 20);
            this.Rho4.TabIndex = 9;
            this.Rho4.Text = "1";
            this.Rho4.Validated += new System.EventHandler(this.Variable_Changed);
            this.Rho4.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Sigma5
            // 
            this.Sigma5.Location = new System.Drawing.Point(286, 146);
            this.Sigma5.Name = "Sigma5";
            this.Sigma5.Size = new System.Drawing.Size(76, 20);
            this.Sigma5.TabIndex = 14;
            this.Sigma5.Text = "3";
            this.Sigma5.Validated += new System.EventHandler(this.Variable_Changed);
            this.Sigma5.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Sigma3
            // 
            this.Sigma3.Location = new System.Drawing.Point(286, 96);
            this.Sigma3.Name = "Sigma3";
            this.Sigma3.Size = new System.Drawing.Size(76, 20);
            this.Sigma3.TabIndex = 8;
            this.Sigma3.Text = "3";
            this.Sigma3.Validated += new System.EventHandler(this.Variable_Changed);
            this.Sigma3.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // LLength3
            // 
            this.LLength3.Location = new System.Drawing.Point(190, 96);
            this.LLength3.Name = "LLength3";
            this.LLength3.Size = new System.Drawing.Size(76, 20);
            this.LLength3.TabIndex = 7;
            this.LLength3.Text = "10";
            this.LLength3.Validated += new System.EventHandler(this.Variable_Changed);
            this.LLength3.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Rho3
            // 
            this.Rho3.Location = new System.Drawing.Point(88, 96);
            this.Rho3.Name = "Rho3";
            this.Rho3.Size = new System.Drawing.Size(76, 20);
            this.Rho3.TabIndex = 6;
            this.Rho3.Text = "1";
            this.Rho3.Validated += new System.EventHandler(this.Variable_Changed);
            this.Rho3.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Sigma2
            // 
            this.Sigma2.Location = new System.Drawing.Point(286, 71);
            this.Sigma2.Name = "Sigma2";
            this.Sigma2.Size = new System.Drawing.Size(76, 20);
            this.Sigma2.TabIndex = 5;
            this.Sigma2.Text = "3";
            this.Sigma2.Validated += new System.EventHandler(this.Variable_Changed);
            this.Sigma2.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // LLength2
            // 
            this.LLength2.Location = new System.Drawing.Point(190, 71);
            this.LLength2.Name = "LLength2";
            this.LLength2.Size = new System.Drawing.Size(76, 20);
            this.LLength2.TabIndex = 4;
            this.LLength2.Text = "10";
            this.LLength2.Validated += new System.EventHandler(this.Variable_Changed);
            this.LLength2.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Rho2
            // 
            this.Rho2.Location = new System.Drawing.Point(88, 71);
            this.Rho2.Name = "Rho2";
            this.Rho2.Size = new System.Drawing.Size(76, 20);
            this.Rho2.TabIndex = 3;
            this.Rho2.Text = "1.3";
            this.Rho2.Validated += new System.EventHandler(this.Variable_Changed);
            this.Rho2.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Sigma1
            // 
            this.Sigma1.Location = new System.Drawing.Point(286, 42);
            this.Sigma1.Name = "Sigma1";
            this.Sigma1.Size = new System.Drawing.Size(76, 20);
            this.Sigma1.TabIndex = 2;
            this.Sigma1.Text = "3";
            this.Sigma1.Validated += new System.EventHandler(this.Variable_Changed);
            this.Sigma1.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // LLength1
            // 
            this.LLength1.Location = new System.Drawing.Point(190, 42);
            this.LLength1.Name = "LLength1";
            this.LLength1.Size = new System.Drawing.Size(76, 20);
            this.LLength1.TabIndex = 1;
            this.LLength1.Text = "16";
            this.LLength1.Validated += new System.EventHandler(this.Variable_Changed);
            this.LLength1.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Rho1
            // 
            this.Rho1.Location = new System.Drawing.Point(88, 42);
            this.Rho1.Name = "Rho1";
            this.Rho1.Size = new System.Drawing.Size(76, 20);
            this.Rho1.TabIndex = 0;
            this.Rho1.Text = ".95";
            this.Rho1.Validated += new System.EventHandler(this.Variable_Changed);
            this.Rho1.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // label11
            // 
            this.label11.AutoSize = true;
            this.label11.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, ((System.Drawing.FontStyle)((System.Drawing.FontStyle.Bold | System.Drawing.FontStyle.Underline))), System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label11.Location = new System.Drawing.Point(283, 16);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(70, 13);
            this.label11.TabIndex = 10;
            this.label11.Text = "Roughness";
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, ((System.Drawing.FontStyle)((System.Drawing.FontStyle.Bold | System.Drawing.FontStyle.Underline))), System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label9.Location = new System.Drawing.Point(187, 16);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(65, 13);
            this.label9.TabIndex = 9;
            this.label9.Text = "Thickness";
            // 
            // RhoLabel
            // 
            this.RhoLabel.AutoSize = true;
            this.RhoLabel.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, ((System.Drawing.FontStyle)((System.Drawing.FontStyle.Bold | System.Drawing.FontStyle.Underline))), System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.RhoLabel.Location = new System.Drawing.Point(85, 16);
            this.RhoLabel.Name = "RhoLabel";
            this.RhoLabel.Size = new System.Drawing.Size(96, 13);
            this.RhoLabel.TabIndex = 8;
            this.RhoLabel.Text = "Normalized Rho";
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, ((System.Drawing.FontStyle)((System.Drawing.FontStyle.Bold | System.Drawing.FontStyle.Underline))), System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label8.Location = new System.Drawing.Point(15, 16);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(38, 13);
            this.label8.TabIndex = 6;
            this.label8.Text = "Layer";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(25, 174);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(13, 13);
            this.label7.TabIndex = 5;
            this.label7.Text = "6";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(25, 149);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(13, 13);
            this.label6.TabIndex = 4;
            this.label6.Text = "5";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(25, 124);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(13, 13);
            this.label5.TabIndex = 3;
            this.label5.Text = "4";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(25, 99);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(13, 13);
            this.label4.TabIndex = 2;
            this.label4.Text = "3";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(25, 74);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(13, 13);
            this.label3.TabIndex = 1;
            this.label3.Text = "2";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(25, 49);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(13, 13);
            this.label2.TabIndex = 0;
            this.label2.Text = "1";
            // 
            // pictureBox1
            // 
            this.pictureBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.pictureBox1.Image = ((System.Drawing.Image)(resources.GetObject("pictureBox1.Image")));
            this.pictureBox1.Location = new System.Drawing.Point(656, 369);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(172, 183);
            this.pictureBox1.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.pictureBox1.TabIndex = 9;
            this.pictureBox1.TabStop = false;
            // 
            // Holdsigma
            // 
            this.Holdsigma.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.Holdsigma.AutoSize = true;
            this.Holdsigma.Checked = true;
            this.Holdsigma.CheckState = System.Windows.Forms.CheckState.Checked;
            this.Holdsigma.Location = new System.Drawing.Point(179, 61);
            this.Holdsigma.Name = "Holdsigma";
            this.Holdsigma.Size = new System.Drawing.Size(103, 17);
            this.Holdsigma.TabIndex = 6;
            this.Holdsigma.Text = "Link Roughness";
            this.Holdsigma.UseVisualStyleBackColor = true;
            this.Holdsigma.CheckedChanged += new System.EventHandler(this.Variable_Changed);
            // 
            // label12
            // 
            this.label12.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.label12.AutoSize = true;
            this.label12.Enabled = false;
            this.label12.Location = new System.Drawing.Point(18, 179);
            this.label12.Name = "label12";
            this.label12.Size = new System.Drawing.Size(79, 13);
            this.label12.TabIndex = 13;
            this.label12.Text = "Subphase SLD";
            // 
            // SubphaseSLD
            // 
            this.SubphaseSLD.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.SubphaseSLD.Location = new System.Drawing.Point(24, 195);
            this.SubphaseSLD.Name = "SubphaseSLD";
            this.SubphaseSLD.Size = new System.Drawing.Size(73, 20);
            this.SubphaseSLD.TabIndex = 3;
            this.SubphaseSLD.Text = "9.38";
            this.SubphaseSLD.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.SubphaseSLD.Validated += new System.EventHandler(this.MajorVariable_Changed);
            this.SubphaseSLD.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // label13
            // 
            this.label13.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.label13.AutoSize = true;
            this.label13.Enabled = false;
            this.label13.Location = new System.Drawing.Point(21, 69);
            this.label13.Name = "label13";
            this.label13.Size = new System.Drawing.Size(112, 13);
            this.label13.TabIndex = 15;
            this.label13.Text = "Subphase Roughness";
            // 
            // UndoFit
            // 
            this.UndoFit.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.UndoFit.Location = new System.Drawing.Point(467, 450);
            this.UndoFit.Name = "UndoFit";
            this.UndoFit.Size = new System.Drawing.Size(108, 24);
            this.UndoFit.TabIndex = 4;
            this.UndoFit.Text = "Undo Fit";
            this.UndoFit.UseVisualStyleBackColor = true;
            this.UndoFit.Click += new System.EventHandler(this.UndoFit_Click);
            // 
            // label15
            // 
            this.label15.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.label15.AutoSize = true;
            this.label15.Location = new System.Drawing.Point(464, 524);
            this.label15.Name = "label15";
            this.label15.Size = new System.Drawing.Size(59, 13);
            this.label15.TabIndex = 20;
            this.label15.Text = "Chi Square";
            // 
            // chisquaretb
            // 
            this.chisquaretb.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.chisquaretb.Enabled = false;
            this.chisquaretb.Location = new System.Drawing.Point(467, 541);
            this.chisquaretb.Name = "chisquaretb";
            this.chisquaretb.Size = new System.Drawing.Size(93, 20);
            this.chisquaretb.TabIndex = 19;
            this.chisquaretb.TabStop = false;
            this.chisquaretb.Text = "0";
            this.chisquaretb.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            // 
            // SubRough
            // 
            this.SubRough.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.SubRough.Location = new System.Drawing.Point(24, 84);
            this.SubRough.Name = "SubRough";
            this.SubRough.Size = new System.Drawing.Size(73, 20);
            this.SubRough.TabIndex = 1;
            this.SubRough.Text = "3.25";
            this.SubRough.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.SubRough.Validated += new System.EventHandler(this.Variable_Changed);
            this.SubRough.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // label14
            // 
            this.label14.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.label14.AutoSize = true;
            this.label14.Enabled = false;
            this.label14.Location = new System.Drawing.Point(21, 124);
            this.label14.Name = "label14";
            this.label14.Size = new System.Drawing.Size(97, 13);
            this.label14.TabIndex = 23;
            this.label14.Text = "Critical Edge Offset";
            // 
            // CritOffset
            // 
            this.CritOffset.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.CritOffset.Location = new System.Drawing.Point(24, 139);
            this.CritOffset.Name = "CritOffset";
            this.CritOffset.Size = new System.Drawing.Size(72, 20);
            this.CritOffset.TabIndex = 2;
            this.CritOffset.Text = "0";
            this.CritOffset.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.CritOffset.Validated += new System.EventHandler(this.LowQ_TextChanged);
            this.CritOffset.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateIntegerInput);
            // 
            // label17
            // 
            this.label17.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.label17.AutoSize = true;
            this.label17.Enabled = false;
            this.label17.Location = new System.Drawing.Point(179, 124);
            this.label17.Name = "label17";
            this.label17.Size = new System.Drawing.Size(69, 13);
            this.label17.TabIndex = 27;
            this.label17.Text = "High Q offset";
            // 
            // Rightoffset
            // 
            this.Rightoffset.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.Rightoffset.Location = new System.Drawing.Point(184, 139);
            this.Rightoffset.Name = "Rightoffset";
            this.Rightoffset.Size = new System.Drawing.Size(72, 20);
            this.Rightoffset.TabIndex = 7;
            this.Rightoffset.Text = "0";
            this.Rightoffset.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.Rightoffset.Validated += new System.EventHandler(this.HQ_TextChanged);
            this.Rightoffset.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateIntegerInput);
            // 
            // button1
            // 
            this.button1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.button1.Location = new System.Drawing.Point(467, 492);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(108, 24);
            this.button1.TabIndex = 5;
            this.button1.Text = "Fit Details";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // button2
            // 
            this.button2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.button2.Location = new System.Drawing.Point(467, 408);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(108, 24);
            this.button2.TabIndex = 3;
            this.button2.Text = "Stochastic Fit";
            this.button2.UseVisualStyleBackColor = true;
            this.button2.Click += new System.EventHandler(this.button2_Click);
            // 
            // loadingCircle1
            // 
            this.loadingCircle1.Active = false;
            this.loadingCircle1.Anchor = System.Windows.Forms.AnchorStyles.None;
            this.loadingCircle1.BackColor = System.Drawing.Color.White;
            this.loadingCircle1.Color = System.Drawing.Color.CornflowerBlue;
            this.loadingCircle1.InnerCircleRadius = 5;
            this.loadingCircle1.Location = new System.Drawing.Point(3, 3);
            this.loadingCircle1.Name = "loadingCircle1";
            this.loadingCircle1.NumberSpoke = 12;
            this.loadingCircle1.OuterCircleRadius = 11;
            this.loadingCircle1.RotationSpeed = 150;
            this.loadingCircle1.Size = new System.Drawing.Size(401, 340);
            this.loadingCircle1.SpokeThickness = 2;
            this.loadingCircle1.StylePreset = MRG.Controls.UI.LoadingCircle.StylePresets.MacOSX;
            this.loadingCircle1.TabIndex = 31;
            this.loadingCircle1.Text = "loadingCircle1";
            // 
            // label16
            // 
            this.label16.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.label16.AutoSize = true;
            this.label16.Enabled = false;
            this.label16.Location = new System.Drawing.Point(179, 180);
            this.label16.Name = "label16";
            this.label16.Size = new System.Drawing.Size(88, 13);
            this.label16.TabIndex = 33;
            this.label16.Text = "Superphase SLD";
            // 
            // SupSLDTB
            // 
            this.SupSLDTB.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.SupSLDTB.Location = new System.Drawing.Point(183, 195);
            this.SupSLDTB.Name = "SupSLDTB";
            this.SupSLDTB.Size = new System.Drawing.Size(73, 20);
            this.SupSLDTB.TabIndex = 8;
            this.SupSLDTB.Text = "0";
            this.SupSLDTB.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.SupSLDTB.Validated += new System.EventHandler(this.MajorVariable_Changed);
            this.SupSLDTB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // label18
            // 
            this.label18.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.label18.AutoSize = true;
            this.label18.Enabled = false;
            this.label18.Location = new System.Drawing.Point(18, 238);
            this.label18.Name = "label18";
            this.label18.Size = new System.Drawing.Size(106, 13);
            this.label18.TabIndex = 35;
            this.label18.Text = "% error in Q (neutron)";
            // 
            // QSpreadTB
            // 
            this.QSpreadTB.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.QSpreadTB.Location = new System.Drawing.Point(24, 253);
            this.QSpreadTB.Name = "QSpreadTB";
            this.QSpreadTB.Size = new System.Drawing.Size(73, 20);
            this.QSpreadTB.TabIndex = 4;
            this.QSpreadTB.Text = "0";
            this.QSpreadTB.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.QSpreadTB.Validated += new System.EventHandler(this.Variable_Changed);
            this.QSpreadTB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateIntegerInput);
            // 
            // DBFCB
            // 
            this.DBFCB.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.DBFCB.AutoSize = true;
            this.DBFCB.Checked = true;
            this.DBFCB.CheckState = System.Windows.Forms.CheckState.Checked;
            this.DBFCB.Location = new System.Drawing.Point(6, 302);
            this.DBFCB.Name = "DBFCB";
            this.DBFCB.Size = new System.Drawing.Size(162, 17);
            this.DBFCB.TabIndex = 36;
            this.DBFCB.Text = "Divide Reflectivity by Fresnel";
            this.DBFCB.UseVisualStyleBackColor = true;
            this.DBFCB.CheckedChanged += new System.EventHandler(this.MajorVariable_Changed);
            // 
            // NormCorrectCB
            // 
            this.NormCorrectCB.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.NormCorrectCB.AutoSize = true;
            this.NormCorrectCB.Location = new System.Drawing.Point(179, 24);
            this.NormCorrectCB.Name = "NormCorrectCB";
            this.NormCorrectCB.Size = new System.Drawing.Size(185, 17);
            this.NormCorrectCB.TabIndex = 5;
            this.NormCorrectCB.Text = "Correct for imperfect normalization";
            this.NormCorrectCB.UseVisualStyleBackColor = true;
            this.NormCorrectCB.CheckedChanged += new System.EventHandler(this.Variable_Changed);
            // 
            // label19
            // 
            this.label19.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.label19.AutoSize = true;
            this.label19.Enabled = false;
            this.label19.Location = new System.Drawing.Point(176, 238);
            this.label19.Name = "label19";
            this.label19.Size = new System.Drawing.Size(121, 13);
            this.label19.TabIndex = 39;
            this.label19.Text = "Normalization Correction";
            // 
            // NormCorrectTB
            // 
            this.NormCorrectTB.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.NormCorrectTB.Location = new System.Drawing.Point(182, 253);
            this.NormCorrectTB.Name = "NormCorrectTB";
            this.NormCorrectTB.Size = new System.Drawing.Size(73, 20);
            this.NormCorrectTB.TabIndex = 9;
            this.NormCorrectTB.Text = "1";
            this.NormCorrectTB.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.NormCorrectTB.Validated += new System.EventHandler(this.Variable_Changed);
            this.NormCorrectTB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // label20
            // 
            this.label20.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.label20.AutoSize = true;
            this.label20.Enabled = false;
            this.label20.Location = new System.Drawing.Point(179, 287);
            this.label20.Name = "label20";
            this.label20.Size = new System.Drawing.Size(65, 13);
            this.label20.TabIndex = 41;
            this.label20.Text = "Wavelength";
            // 
            // WavelengthTB
            // 
            this.WavelengthTB.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.WavelengthTB.Location = new System.Drawing.Point(184, 302);
            this.WavelengthTB.Name = "WavelengthTB";
            this.WavelengthTB.Size = new System.Drawing.Size(72, 20);
            this.WavelengthTB.TabIndex = 10;
            this.WavelengthTB.Text = "1.24";
            this.WavelengthTB.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.WavelengthTB.Validated += new System.EventHandler(this.MajorVariable_Changed);
            this.WavelengthTB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // tabControl1
            // 
            this.tabControl1.Controls.Add(this.tabPage1);
            this.tabControl1.Controls.Add(this.tabPage2);
            this.tabControl1.Location = new System.Drawing.Point(12, 8);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(422, 382);
            this.tabControl1.TabIndex = 1;
            // 
            // tabPage1
            // 
            this.tabPage1.Controls.Add(this.loadingCircle1);
            this.tabPage1.Controls.Add(this.RhoGraph);
            this.tabPage1.Location = new System.Drawing.Point(4, 22);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage1.Size = new System.Drawing.Size(414, 356);
            this.tabPage1.TabIndex = 0;
            this.tabPage1.Text = "Reflectivity";
            this.tabPage1.UseVisualStyleBackColor = true;
            // 
            // tabPage2
            // 
            this.tabPage2.Controls.Add(this.loadingCircle2);
            this.tabPage2.Controls.Add(this.EDzedGraphControl1);
            this.tabPage2.Location = new System.Drawing.Point(4, 22);
            this.tabPage2.Name = "tabPage2";
            this.tabPage2.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage2.Size = new System.Drawing.Size(414, 356);
            this.tabPage2.TabIndex = 1;
            this.tabPage2.Text = "Electron Density";
            this.tabPage2.UseVisualStyleBackColor = true;
            // 
            // loadingCircle2
            // 
            this.loadingCircle2.Active = false;
            this.loadingCircle2.Anchor = System.Windows.Forms.AnchorStyles.None;
            this.loadingCircle2.BackColor = System.Drawing.Color.White;
            this.loadingCircle2.Color = System.Drawing.Color.CornflowerBlue;
            this.loadingCircle2.InnerCircleRadius = 5;
            this.loadingCircle2.Location = new System.Drawing.Point(2, 6);
            this.loadingCircle2.Name = "loadingCircle2";
            this.loadingCircle2.NumberSpoke = 12;
            this.loadingCircle2.OuterCircleRadius = 11;
            this.loadingCircle2.RotationSpeed = 150;
            this.loadingCircle2.Size = new System.Drawing.Size(406, 340);
            this.loadingCircle2.SpokeThickness = 2;
            this.loadingCircle2.StylePreset = MRG.Controls.UI.LoadingCircle.StylePresets.MacOSX;
            this.loadingCircle2.TabIndex = 32;
            this.loadingCircle2.Text = "loadingCircle2";
            // 
            // EDzedGraphControl1
            // 
            this.EDzedGraphControl1.Anchor = System.Windows.Forms.AnchorStyles.None;
            this.EDzedGraphControl1.AutoSize = true;
            this.EDzedGraphControl1.EditButtons = System.Windows.Forms.MouseButtons.Left;
            this.EDzedGraphControl1.Location = new System.Drawing.Point(3, 6);
            this.EDzedGraphControl1.Name = "EDzedGraphControl1";
            this.EDzedGraphControl1.PanModifierKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Shift | System.Windows.Forms.Keys.None)));
            this.EDzedGraphControl1.ScrollGrace = 0;
            this.EDzedGraphControl1.ScrollMaxX = 0;
            this.EDzedGraphControl1.ScrollMaxY = 0;
            this.EDzedGraphControl1.ScrollMaxY2 = 0;
            this.EDzedGraphControl1.ScrollMinX = 0;
            this.EDzedGraphControl1.ScrollMinY = 0;
            this.EDzedGraphControl1.ScrollMinY2 = 0;
            this.EDzedGraphControl1.Size = new System.Drawing.Size(401, 340);
            this.EDzedGraphControl1.TabIndex = 43;
            // 
            // controlbox
            // 
            this.controlbox.Controls.Add(this.label20);
            this.controlbox.Controls.Add(this.WavelengthTB);
            this.controlbox.Controls.Add(this.label19);
            this.controlbox.Controls.Add(this.NormCorrectTB);
            this.controlbox.Controls.Add(this.NormCorrectCB);
            this.controlbox.Controls.Add(this.DBFCB);
            this.controlbox.Controls.Add(this.label18);
            this.controlbox.Controls.Add(this.QSpreadTB);
            this.controlbox.Controls.Add(this.label16);
            this.controlbox.Controls.Add(this.SupSLDTB);
            this.controlbox.Controls.Add(this.label17);
            this.controlbox.Controls.Add(this.Rightoffset);
            this.controlbox.Controls.Add(this.label14);
            this.controlbox.Controls.Add(this.CritOffset);
            this.controlbox.Controls.Add(this.label13);
            this.controlbox.Controls.Add(this.SubRough);
            this.controlbox.Controls.Add(this.label12);
            this.controlbox.Controls.Add(this.SubphaseSLD);
            this.controlbox.Controls.Add(this.Holdsigma);
            this.controlbox.Controls.Add(this.label1);
            this.controlbox.Controls.Add(this.BoxCount);
            this.controlbox.Location = new System.Drawing.Point(443, 8);
            this.controlbox.Name = "controlbox";
            this.controlbox.Size = new System.Drawing.Size(384, 337);
            this.controlbox.TabIndex = 0;
            this.controlbox.TabStop = false;
            // 
            // Reflmodeling
            // 
            this.AcceptButton = this.LevenbergFit;
            this.AutoScaleDimensions = new System.Drawing.SizeF(96F, 96F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Dpi;
            this.ClientSize = new System.Drawing.Size(852, 618);
            this.Controls.Add(this.controlbox);
            this.Controls.Add(this.tabControl1);
            this.Controls.Add(this.button2);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.label15);
            this.Controls.Add(this.chisquaretb);
            this.Controls.Add(this.UndoFit);
            this.Controls.Add(this.pictureBox1);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.LevenbergFit);
            this.DoubleBuffered = true;
            this.MaximizeBox = false;
            this.Name = "Reflmodeling";
            this.Text = "Reflectivity Modeling";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.OnFormClosing);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.tabControl1.ResumeLayout(false);
            this.tabPage1.ResumeLayout(false);
            this.tabPage1.PerformLayout();
            this.tabPage2.ResumeLayout(false);
            this.tabPage2.PerformLayout();
            this.controlbox.ResumeLayout(false);
            this.controlbox.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private ZedGraph.ZedGraphControl RhoGraph;
        private System.Windows.Forms.Button LevenbergFit;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox BoxCount;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label RhoLabel;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.Label label11;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.TextBox LLength5;
        private System.Windows.Forms.TextBox Sigma4;
        private System.Windows.Forms.TextBox LLength4;
        private System.Windows.Forms.TextBox Rho5;
        private System.Windows.Forms.TextBox Rho4;
        private System.Windows.Forms.TextBox Sigma5;
        private System.Windows.Forms.TextBox Sigma3;
        private System.Windows.Forms.TextBox LLength3;
        private System.Windows.Forms.TextBox Rho3;
        private System.Windows.Forms.TextBox Sigma2;
        private System.Windows.Forms.TextBox LLength2;
        private System.Windows.Forms.TextBox Rho2;
        private System.Windows.Forms.TextBox Sigma1;
        private System.Windows.Forms.TextBox LLength1;
        private System.Windows.Forms.TextBox Rho1;
        private System.Windows.Forms.TextBox Rho6;
        private System.Windows.Forms.TextBox LLength6;
        private System.Windows.Forms.TextBox Sigma6;
        private System.Windows.Forms.CheckBox Holdsigma;
        private System.Windows.Forms.Label label12;
        private System.Windows.Forms.TextBox SubphaseSLD;
        private System.Windows.Forms.Label label13;
        private System.Windows.Forms.Button UndoFit;
        private System.Windows.Forms.Label label15;
        private System.Windows.Forms.TextBox chisquaretb;
        private System.Windows.Forms.TextBox SubRough;
        private System.Windows.Forms.Label label14;
        private System.Windows.Forms.TextBox CritOffset;
        private System.Windows.Forms.Label label17;
        private System.Windows.Forms.TextBox Rightoffset;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.Button button2;
        private MRG.Controls.UI.LoadingCircle loadingCircle1;
        private System.Windows.Forms.Label label16;
        private System.Windows.Forms.TextBox SupSLDTB;
        private System.Windows.Forms.Label label18;
        private System.Windows.Forms.TextBox QSpreadTB;
        private System.Windows.Forms.CheckBox DBFCB;
        private System.Windows.Forms.CheckBox NormCorrectCB;
        private System.Windows.Forms.Label label19;
        private System.Windows.Forms.TextBox NormCorrectTB;
        private System.Windows.Forms.Label label20;
        private System.Windows.Forms.TextBox WavelengthTB;
        private System.Windows.Forms.TabControl tabControl1;
        private System.Windows.Forms.TabPage tabPage1;
        private System.Windows.Forms.TabPage tabPage2;
        private MRG.Controls.UI.LoadingCircle loadingCircle2;
        private ZedGraph.ZedGraphControl EDzedGraphControl1;
        private System.Windows.Forms.GroupBox controlbox;
    }
}