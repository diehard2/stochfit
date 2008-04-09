namespace StochasticModeling
{
    partial class Rhomodeling
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Rhomodeling));
            this.LevenbergFit = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.BoxCount = new System.Windows.Forms.TextBox();
            this.ParameterGB = new System.Windows.Forms.GroupBox();
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
            this.SubRough = new System.Windows.Forms.TextBox();
            this.label14 = new System.Windows.Forms.Label();
            this.Zoffset = new System.Windows.Forms.TextBox();
            this.UndoFit = new System.Windows.Forms.Button();
            this.label15 = new System.Windows.Forms.Label();
            this.chisquaretb = new System.Windows.Forms.TextBox();
            this.Report_btn = new System.Windows.Forms.Button();
            this.button1 = new System.Windows.Forms.Button();
            this.RhoGraph = new ZedGraph.ZedGraphControl();
            this.label16 = new System.Windows.Forms.Label();
            this.SupSLDTB = new System.Windows.Forms.TextBox();
            this.ParameterGB.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.SuspendLayout();
            // 
            // LevenbergFit
            // 
            this.LevenbergFit.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.LevenbergFit.Location = new System.Drawing.Point(486, 382);
            this.LevenbergFit.Name = "LevenbergFit";
            this.LevenbergFit.Size = new System.Drawing.Size(108, 24);
            this.LevenbergFit.TabIndex = 6;
            this.LevenbergFit.Text = "Fit";
            this.LevenbergFit.UseVisualStyleBackColor = true;
            this.LevenbergFit.Click += new System.EventHandler(this.LevenbergFit_Click);
            // 
            // label1
            // 
            this.label1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.label1.AutoSize = true;
            this.label1.Enabled = false;
            this.label1.Location = new System.Drawing.Point(464, 19);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(88, 13);
            this.label1.TabIndex = 7;
            this.label1.Text = "Number of Boxes";
            // 
            // BoxCount
            // 
            this.BoxCount.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.BoxCount.Location = new System.Drawing.Point(467, 35);
            this.BoxCount.Name = "BoxCount";
            this.BoxCount.Size = new System.Drawing.Size(26, 20);
            this.BoxCount.TabIndex = 1;
            this.BoxCount.Text = "2";
            this.BoxCount.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.BoxCount.Validated += new System.EventHandler(this.Field_Validated);
            this.BoxCount.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateIntegerInput);
            // 
            // ParameterGB
            // 
            this.ParameterGB.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.ParameterGB.Controls.Add(this.Rho6);
            this.ParameterGB.Controls.Add(this.LLength6);
            this.ParameterGB.Controls.Add(this.Sigma6);
            this.ParameterGB.Controls.Add(this.LLength5);
            this.ParameterGB.Controls.Add(this.Sigma4);
            this.ParameterGB.Controls.Add(this.LLength4);
            this.ParameterGB.Controls.Add(this.Rho5);
            this.ParameterGB.Controls.Add(this.Rho4);
            this.ParameterGB.Controls.Add(this.Sigma5);
            this.ParameterGB.Controls.Add(this.Sigma3);
            this.ParameterGB.Controls.Add(this.LLength3);
            this.ParameterGB.Controls.Add(this.Rho3);
            this.ParameterGB.Controls.Add(this.Sigma2);
            this.ParameterGB.Controls.Add(this.LLength2);
            this.ParameterGB.Controls.Add(this.Rho2);
            this.ParameterGB.Controls.Add(this.Sigma1);
            this.ParameterGB.Controls.Add(this.LLength1);
            this.ParameterGB.Controls.Add(this.Rho1);
            this.ParameterGB.Controls.Add(this.label11);
            this.ParameterGB.Controls.Add(this.label9);
            this.ParameterGB.Controls.Add(this.RhoLabel);
            this.ParameterGB.Controls.Add(this.label8);
            this.ParameterGB.Controls.Add(this.label7);
            this.ParameterGB.Controls.Add(this.label6);
            this.ParameterGB.Controls.Add(this.label5);
            this.ParameterGB.Controls.Add(this.label4);
            this.ParameterGB.Controls.Add(this.label3);
            this.ParameterGB.Controls.Add(this.label2);
            this.ParameterGB.Location = new System.Drawing.Point(12, 359);
            this.ParameterGB.Name = "ParameterGB";
            this.ParameterGB.Size = new System.Drawing.Size(392, 203);
            this.ParameterGB.TabIndex = 5;
            this.ParameterGB.TabStop = false;
            this.ParameterGB.Text = "Layer Parameters";
            // 
            // Rho6
            // 
            this.Rho6.Location = new System.Drawing.Point(88, 171);
            this.Rho6.Name = "Rho6";
            this.Rho6.Size = new System.Drawing.Size(76, 20);
            this.Rho6.TabIndex = 15;
            this.Rho6.Text = "1";
            this.Rho6.Validated += new System.EventHandler(this.Field_Validated);
            this.Rho6.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // LLength6
            // 
            this.LLength6.Location = new System.Drawing.Point(190, 171);
            this.LLength6.Name = "LLength6";
            this.LLength6.Size = new System.Drawing.Size(76, 20);
            this.LLength6.TabIndex = 16;
            this.LLength6.Text = "10";
            this.LLength6.Validated += new System.EventHandler(this.Field_Validated);
            this.LLength6.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Sigma6
            // 
            this.Sigma6.Location = new System.Drawing.Point(286, 171);
            this.Sigma6.Name = "Sigma6";
            this.Sigma6.Size = new System.Drawing.Size(76, 20);
            this.Sigma6.TabIndex = 17;
            this.Sigma6.Text = "3";
            this.Sigma6.Validated += new System.EventHandler(this.Field_Validated);
            this.Sigma6.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // LLength5
            // 
            this.LLength5.Location = new System.Drawing.Point(190, 146);
            this.LLength5.Name = "LLength5";
            this.LLength5.Size = new System.Drawing.Size(76, 20);
            this.LLength5.TabIndex = 13;
            this.LLength5.Text = "10";
            this.LLength5.Validated += new System.EventHandler(this.Field_Validated);
            this.LLength5.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Sigma4
            // 
            this.Sigma4.Location = new System.Drawing.Point(286, 121);
            this.Sigma4.Name = "Sigma4";
            this.Sigma4.Size = new System.Drawing.Size(76, 20);
            this.Sigma4.TabIndex = 11;
            this.Sigma4.Text = "3";
            this.Sigma4.Validated += new System.EventHandler(this.Field_Validated);
            this.Sigma4.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // LLength4
            // 
            this.LLength4.Location = new System.Drawing.Point(190, 121);
            this.LLength4.Name = "LLength4";
            this.LLength4.Size = new System.Drawing.Size(76, 20);
            this.LLength4.TabIndex = 10;
            this.LLength4.Text = "10";
            this.LLength4.Validated += new System.EventHandler(this.Field_Validated);
            this.LLength4.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Rho5
            // 
            this.Rho5.Location = new System.Drawing.Point(88, 146);
            this.Rho5.Name = "Rho5";
            this.Rho5.Size = new System.Drawing.Size(76, 20);
            this.Rho5.TabIndex = 12;
            this.Rho5.Text = "1";
            this.Rho5.Validated += new System.EventHandler(this.Field_Validated);
            this.Rho5.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Rho4
            // 
            this.Rho4.Location = new System.Drawing.Point(88, 121);
            this.Rho4.Name = "Rho4";
            this.Rho4.Size = new System.Drawing.Size(76, 20);
            this.Rho4.TabIndex = 9;
            this.Rho4.Text = "1";
            this.Rho4.Validated += new System.EventHandler(this.Field_Validated);
            this.Rho4.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Sigma5
            // 
            this.Sigma5.Location = new System.Drawing.Point(286, 146);
            this.Sigma5.Name = "Sigma5";
            this.Sigma5.Size = new System.Drawing.Size(76, 20);
            this.Sigma5.TabIndex = 14;
            this.Sigma5.Text = "3";
            this.Sigma5.Validated += new System.EventHandler(this.Field_Validated);
            this.Sigma5.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Sigma3
            // 
            this.Sigma3.Location = new System.Drawing.Point(286, 96);
            this.Sigma3.Name = "Sigma3";
            this.Sigma3.Size = new System.Drawing.Size(76, 20);
            this.Sigma3.TabIndex = 8;
            this.Sigma3.Text = "3";
            this.Sigma3.Validated += new System.EventHandler(this.Field_Validated);
            this.Sigma3.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // LLength3
            // 
            this.LLength3.Location = new System.Drawing.Point(190, 96);
            this.LLength3.Name = "LLength3";
            this.LLength3.Size = new System.Drawing.Size(76, 20);
            this.LLength3.TabIndex = 7;
            this.LLength3.Text = "10";
            this.LLength3.Validated += new System.EventHandler(this.Field_Validated);
            this.LLength3.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Rho3
            // 
            this.Rho3.Location = new System.Drawing.Point(88, 96);
            this.Rho3.Name = "Rho3";
            this.Rho3.Size = new System.Drawing.Size(76, 20);
            this.Rho3.TabIndex = 6;
            this.Rho3.Text = "1";
            this.Rho3.Validated += new System.EventHandler(this.Field_Validated);
            this.Rho3.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Sigma2
            // 
            this.Sigma2.Location = new System.Drawing.Point(286, 71);
            this.Sigma2.Name = "Sigma2";
            this.Sigma2.Size = new System.Drawing.Size(76, 20);
            this.Sigma2.TabIndex = 5;
            this.Sigma2.Text = "3";
            this.Sigma2.Validated += new System.EventHandler(this.Field_Validated);
            this.Sigma2.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // LLength2
            // 
            this.LLength2.Location = new System.Drawing.Point(190, 71);
            this.LLength2.Name = "LLength2";
            this.LLength2.Size = new System.Drawing.Size(76, 20);
            this.LLength2.TabIndex = 4;
            this.LLength2.Text = "10";
            this.LLength2.Validated += new System.EventHandler(this.Field_Validated);
            this.LLength2.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Rho2
            // 
            this.Rho2.Location = new System.Drawing.Point(88, 71);
            this.Rho2.Name = "Rho2";
            this.Rho2.Size = new System.Drawing.Size(76, 20);
            this.Rho2.TabIndex = 3;
            this.Rho2.Text = "1.3";
            this.Rho2.Validated += new System.EventHandler(this.Field_Validated);
            this.Rho2.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Sigma1
            // 
            this.Sigma1.Location = new System.Drawing.Point(286, 42);
            this.Sigma1.Name = "Sigma1";
            this.Sigma1.Size = new System.Drawing.Size(76, 20);
            this.Sigma1.TabIndex = 2;
            this.Sigma1.Text = "3";
            this.Sigma1.Validated += new System.EventHandler(this.Field_Validated);
            this.Sigma1.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // LLength1
            // 
            this.LLength1.Location = new System.Drawing.Point(190, 42);
            this.LLength1.Name = "LLength1";
            this.LLength1.Size = new System.Drawing.Size(76, 20);
            this.LLength1.TabIndex = 1;
            this.LLength1.Text = "16";
            this.LLength1.Validated += new System.EventHandler(this.Field_Validated);
            this.LLength1.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Rho1
            // 
            this.Rho1.Location = new System.Drawing.Point(88, 42);
            this.Rho1.Name = "Rho1";
            this.Rho1.Size = new System.Drawing.Size(76, 20);
            this.Rho1.TabIndex = 0;
            this.Rho1.Text = ".95";
            this.Rho1.Validated += new System.EventHandler(this.Field_Validated);
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
            this.pictureBox1.Location = new System.Drawing.Point(486, 176);
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
            this.Holdsigma.Location = new System.Drawing.Point(584, 141);
            this.Holdsigma.Name = "Holdsigma";
            this.Holdsigma.Size = new System.Drawing.Size(103, 17);
            this.Holdsigma.TabIndex = 2;
            this.Holdsigma.Text = "Link Roughness";
            this.Holdsigma.UseVisualStyleBackColor = true;
            this.Holdsigma.CheckedChanged += new System.EventHandler(this.Field_Validated);
            // 
            // label12
            // 
            this.label12.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.label12.AutoSize = true;
            this.label12.Enabled = false;
            this.label12.Location = new System.Drawing.Point(461, 65);
            this.label12.Name = "label12";
            this.label12.Size = new System.Drawing.Size(79, 13);
            this.label12.TabIndex = 13;
            this.label12.Text = "Subphase SLD";
            // 
            // SubphaseSLD
            // 
            this.SubphaseSLD.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.SubphaseSLD.Location = new System.Drawing.Point(467, 81);
            this.SubphaseSLD.Name = "SubphaseSLD";
            this.SubphaseSLD.Size = new System.Drawing.Size(73, 20);
            this.SubphaseSLD.TabIndex = 3;
            this.SubphaseSLD.Text = "9.36";
            this.SubphaseSLD.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.SubphaseSLD.Validated += new System.EventHandler(this.Field_Validated);
            this.SubphaseSLD.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // label13
            // 
            this.label13.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.label13.AutoSize = true;
            this.label13.Enabled = false;
            this.label13.Location = new System.Drawing.Point(461, 122);
            this.label13.Name = "label13";
            this.label13.Size = new System.Drawing.Size(112, 13);
            this.label13.TabIndex = 15;
            this.label13.Text = "Subphase Roughness";
            // 
            // SubRough
            // 
            this.SubRough.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.SubRough.Location = new System.Drawing.Point(467, 138);
            this.SubRough.Name = "SubRough";
            this.SubRough.Size = new System.Drawing.Size(73, 20);
            this.SubRough.TabIndex = 4;
            this.SubRough.Text = "3.25";
            this.SubRough.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.SubRough.Validated += new System.EventHandler(this.Field_Validated);
            this.SubRough.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // label14
            // 
            this.label14.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.label14.AutoSize = true;
            this.label14.CausesValidation = false;
            this.label14.Enabled = false;
            this.label14.Location = new System.Drawing.Point(584, 19);
            this.label14.Name = "label14";
            this.label14.Size = new System.Drawing.Size(103, 13);
            this.label14.TabIndex = 17;
            this.label14.Text = "Z Offset (Angstroms)";
            // 
            // Zoffset
            // 
            this.Zoffset.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.Zoffset.Location = new System.Drawing.Point(587, 35);
            this.Zoffset.Name = "Zoffset";
            this.Zoffset.Size = new System.Drawing.Size(41, 20);
            this.Zoffset.TabIndex = 3;
            this.Zoffset.Text = "80";
            this.Zoffset.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.Zoffset.Validated += new System.EventHandler(this.Field_Validated);
            this.Zoffset.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // UndoFit
            // 
            this.UndoFit.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.UndoFit.Location = new System.Drawing.Point(486, 420);
            this.UndoFit.Name = "UndoFit";
            this.UndoFit.Size = new System.Drawing.Size(108, 24);
            this.UndoFit.TabIndex = 7;
            this.UndoFit.Text = "Undo Fit";
            this.UndoFit.UseVisualStyleBackColor = true;
            this.UndoFit.Click += new System.EventHandler(this.UndoFit_Click);
            // 
            // label15
            // 
            this.label15.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.label15.AutoSize = true;
            this.label15.Location = new System.Drawing.Point(611, 369);
            this.label15.Name = "label15";
            this.label15.Size = new System.Drawing.Size(59, 13);
            this.label15.TabIndex = 20;
            this.label15.Text = "Chi Square";
            // 
            // chisquaretb
            // 
            this.chisquaretb.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.chisquaretb.Enabled = false;
            this.chisquaretb.Location = new System.Drawing.Point(614, 385);
            this.chisquaretb.Name = "chisquaretb";
            this.chisquaretb.Size = new System.Drawing.Size(73, 20);
            this.chisquaretb.TabIndex = 19;
            this.chisquaretb.Text = "0";
            this.chisquaretb.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            // 
            // Report_btn
            // 
            this.Report_btn.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.Report_btn.Location = new System.Drawing.Point(486, 458);
            this.Report_btn.Name = "Report_btn";
            this.Report_btn.Size = new System.Drawing.Size(108, 24);
            this.Report_btn.TabIndex = 8;
            this.Report_btn.Text = "Fit details";
            this.Report_btn.UseVisualStyleBackColor = true;
            this.Report_btn.Click += new System.EventHandler(this.Report_btn_Click);
            // 
            // button1
            // 
            this.button1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.button1.Location = new System.Drawing.Point(486, 496);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(108, 24);
            this.button1.TabIndex = 9;
            this.button1.Text = "Reflectivity Fit";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // RhoGraph
            // 
            this.RhoGraph.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.RhoGraph.AutoSize = true;
            this.RhoGraph.EditButtons = System.Windows.Forms.MouseButtons.Left;
            this.RhoGraph.Location = new System.Drawing.Point(12, 3);
            this.RhoGraph.Name = "RhoGraph";
            this.RhoGraph.PanModifierKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Shift | System.Windows.Forms.Keys.None)));
            this.RhoGraph.ScrollGrace = 0;
            this.RhoGraph.ScrollMaxX = 0;
            this.RhoGraph.ScrollMaxY = 0;
            this.RhoGraph.ScrollMaxY2 = 0;
            this.RhoGraph.ScrollMinX = 0;
            this.RhoGraph.ScrollMinY = 0;
            this.RhoGraph.ScrollMinY2 = 0;
            this.RhoGraph.Size = new System.Drawing.Size(439, 340);
            this.RhoGraph.TabIndex = 2;
            // 
            // label16
            // 
            this.label16.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.label16.AutoSize = true;
            this.label16.Enabled = false;
            this.label16.Location = new System.Drawing.Point(581, 65);
            this.label16.Name = "label16";
            this.label16.Size = new System.Drawing.Size(88, 13);
            this.label16.TabIndex = 22;
            this.label16.Text = "Superphase SLD";
            // 
            // SupSLDTB
            // 
            this.SupSLDTB.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.SupSLDTB.Location = new System.Drawing.Point(587, 81);
            this.SupSLDTB.Name = "SupSLDTB";
            this.SupSLDTB.Size = new System.Drawing.Size(73, 20);
            this.SupSLDTB.TabIndex = 21;
            this.SupSLDTB.Text = "0";
            this.SupSLDTB.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.SupSLDTB.Validated += new System.EventHandler(this.Field_Validated);
            this.SupSLDTB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // Rhomodeling
            // 
            this.AcceptButton = this.LevenbergFit;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(693, 574);
            this.Controls.Add(this.label16);
            this.Controls.Add(this.SupSLDTB);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.Report_btn);
            this.Controls.Add(this.label15);
            this.Controls.Add(this.chisquaretb);
            this.Controls.Add(this.UndoFit);
            this.Controls.Add(this.label14);
            this.Controls.Add(this.Zoffset);
            this.Controls.Add(this.label13);
            this.Controls.Add(this.SubRough);
            this.Controls.Add(this.label12);
            this.Controls.Add(this.SubphaseSLD);
            this.Controls.Add(this.Holdsigma);
            this.Controls.Add(this.pictureBox1);
            this.Controls.Add(this.ParameterGB);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.BoxCount);
            this.Controls.Add(this.LevenbergFit);
            this.Controls.Add(this.RhoGraph);
            this.DoubleBuffered = true;
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.Fixed3D;
            this.Name = "Rhomodeling";
            this.Text = "Rho Model";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.OnFormClosing);
            this.ParameterGB.ResumeLayout(false);
            this.ParameterGB.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private ZedGraph.ZedGraphControl RhoGraph;
        private System.Windows.Forms.Button LevenbergFit;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox BoxCount;
        private System.Windows.Forms.GroupBox ParameterGB;
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
        private System.Windows.Forms.TextBox SubRough;
        private System.Windows.Forms.Label label14;
        private System.Windows.Forms.TextBox Zoffset;
        private System.Windows.Forms.Button UndoFit;
        private System.Windows.Forms.Label label15;
        private System.Windows.Forms.TextBox chisquaretb;
        private System.Windows.Forms.Button Report_btn;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.Label label16;
        private System.Windows.Forms.TextBox SupSLDTB;
    }
}