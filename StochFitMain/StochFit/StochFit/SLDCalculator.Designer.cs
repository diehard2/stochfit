namespace StochasticModeling
{
    partial class SLDCalculator
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
            this.HsTB = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.PsTB = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.OsTB = new System.Windows.Forms.TextBox();
            this.label6 = new System.Windows.Forms.Label();
            this.NsTB = new System.Windows.Forms.TextBox();
            this.label7 = new System.Windows.Forms.Label();
            this.CsTB = new System.Windows.Forms.TextBox();
            this.label8 = new System.Windows.Forms.Label();
            this.DsTB = new System.Windows.Forms.TextBox();
            this.UseElemCB = new System.Windows.Forms.CheckBox();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.ElecTB = new System.Windows.Forms.TextBox();
            this.label9 = new System.Windows.Forms.Label();
            this.ApmTB = new System.Windows.Forms.TextBox();
            this.label10 = new System.Windows.Forms.Label();
            this.ThickTB = new System.Windows.Forms.TextBox();
            this.label11 = new System.Windows.Forms.Label();
            this.DensTB = new System.Windows.Forms.TextBox();
            this.label12 = new System.Windows.Forms.Label();
            this.label13 = new System.Windows.Forms.Label();
            this.label14 = new System.Windows.Forms.Label();
            this.SLDTB = new System.Windows.Forms.TextBox();
            this.UseDensCB = new System.Windows.Forms.CheckBox();
            this.CalcSLDBB = new System.Windows.Forms.Button();
            this.NeutronCB = new System.Windows.Forms.CheckBox();
            this.panel1 = new System.Windows.Forms.Panel();
            this.panel2 = new System.Windows.Forms.Panel();
            this.label15 = new System.Windows.Forms.Label();
            this.label16 = new System.Windows.Forms.Label();
            this.SiTB = new System.Windows.Forms.TextBox();
            this.panel1.SuspendLayout();
            this.panel2.SuspendLayout();
            this.SuspendLayout();
            // 
            // HsTB
            // 
            this.HsTB.Location = new System.Drawing.Point(28, 21);
            this.HsTB.Name = "HsTB";
            this.HsTB.Size = new System.Drawing.Size(89, 20);
            this.HsTB.TabIndex = 0;
            this.HsTB.Text = "2";
            this.HsTB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateIntegerInput);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(25, 5);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(58, 13);
            this.label1.TabIndex = 1;
            this.label1.Text = "Hydrogens";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(25, 260);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(63, 13);
            this.label4.TabIndex = 7;
            this.label4.Text = "Phosphorus";
            // 
            // PsTB
            // 
            this.PsTB.Location = new System.Drawing.Point(28, 276);
            this.PsTB.Name = "PsTB";
            this.PsTB.Size = new System.Drawing.Size(89, 20);
            this.PsTB.TabIndex = 6;
            this.PsTB.Text = "0";
            this.PsTB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateIntegerInput);
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(25, 209);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(43, 13);
            this.label5.TabIndex = 9;
            this.label5.Text = "Oxygen";
            // 
            // OsTB
            // 
            this.OsTB.Location = new System.Drawing.Point(28, 225);
            this.OsTB.Name = "OsTB";
            this.OsTB.Size = new System.Drawing.Size(89, 20);
            this.OsTB.TabIndex = 8;
            this.OsTB.Text = "1";
            this.OsTB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateIntegerInput);
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(25, 158);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(47, 13);
            this.label6.TabIndex = 11;
            this.label6.Text = "Nitrogen";
            // 
            // NsTB
            // 
            this.NsTB.Location = new System.Drawing.Point(28, 174);
            this.NsTB.Name = "NsTB";
            this.NsTB.Size = new System.Drawing.Size(89, 20);
            this.NsTB.TabIndex = 10;
            this.NsTB.Text = "0";
            this.NsTB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateIntegerInput);
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(25, 110);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(41, 13);
            this.label7.TabIndex = 13;
            this.label7.Text = "Carbon";
            // 
            // CsTB
            // 
            this.CsTB.Location = new System.Drawing.Point(28, 123);
            this.CsTB.Name = "CsTB";
            this.CsTB.Size = new System.Drawing.Size(89, 20);
            this.CsTB.TabIndex = 12;
            this.CsTB.Text = "0";
            this.CsTB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateIntegerInput);
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(25, 56);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(60, 13);
            this.label8.TabIndex = 15;
            this.label8.Text = "Deuteriums";
            // 
            // DsTB
            // 
            this.DsTB.Location = new System.Drawing.Point(28, 72);
            this.DsTB.Name = "DsTB";
            this.DsTB.Size = new System.Drawing.Size(89, 20);
            this.DsTB.TabIndex = 14;
            this.DsTB.Text = "0";
            this.DsTB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateIntegerInput);
            // 
            // UseElemCB
            // 
            this.UseElemCB.AutoSize = true;
            this.UseElemCB.Location = new System.Drawing.Point(28, 367);
            this.UseElemCB.Name = "UseElemCB";
            this.UseElemCB.Size = new System.Drawing.Size(115, 17);
            this.UseElemCB.TabIndex = 16;
            this.UseElemCB.Text = "Use element count";
            this.UseElemCB.UseVisualStyleBackColor = true;
            this.UseElemCB.CheckedChanged += new System.EventHandler(this.UseElemCB_CheckedChanged);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(157, 135);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(18, 13);
            this.label2.TabIndex = 17;
            this.label2.Text = "Or";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(200, 116);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(102, 13);
            this.label3.TabIndex = 19;
            this.label3.Text = "Number of electrons";
            // 
            // ElecTB
            // 
            this.ElecTB.Location = new System.Drawing.Point(203, 132);
            this.ElecTB.Name = "ElecTB";
            this.ElecTB.Size = new System.Drawing.Size(89, 20);
            this.ElecTB.TabIndex = 18;
            this.ElecTB.Text = "10";
            this.ElecTB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateIntegerInput);
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(19, 184);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(93, 13);
            this.label9.TabIndex = 25;
            this.label9.Text = "Area per Molecule";
            // 
            // ApmTB
            // 
            this.ApmTB.Location = new System.Drawing.Point(22, 213);
            this.ApmTB.Name = "ApmTB";
            this.ApmTB.Size = new System.Drawing.Size(89, 20);
            this.ApmTB.TabIndex = 24;
            this.ApmTB.Text = "0";
            this.ApmTB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Location = new System.Drawing.Point(19, 246);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(114, 13);
            this.label10.TabIndex = 23;
            this.label10.Text = "Thickness (Angstroms)";
            // 
            // ThickTB
            // 
            this.ThickTB.Location = new System.Drawing.Point(22, 262);
            this.ThickTB.Name = "ThickTB";
            this.ThickTB.Size = new System.Drawing.Size(89, 20);
            this.ThickTB.TabIndex = 22;
            this.ThickTB.Text = "0";
            this.ThickTB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // label11
            // 
            this.label11.AutoSize = true;
            this.label11.Location = new System.Drawing.Point(19, 44);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(88, 13);
            this.label11.TabIndex = 21;
            this.label11.Text = "Density (g/cm^3)";
            // 
            // DensTB
            // 
            this.DensTB.Location = new System.Drawing.Point(22, 60);
            this.DensTB.Name = "DensTB";
            this.DensTB.Size = new System.Drawing.Size(89, 20);
            this.DensTB.TabIndex = 20;
            this.DensTB.Text = "0.9957";
            this.DensTB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // label12
            // 
            this.label12.AutoSize = true;
            this.label12.Location = new System.Drawing.Point(19, 197);
            this.label12.Name = "label12";
            this.label12.Size = new System.Drawing.Size(79, 13);
            this.label12.TabIndex = 26;
            this.label12.Text = "(A^2/molecule)";
            // 
            // label13
            // 
            this.label13.AutoSize = true;
            this.label13.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label13.Location = new System.Drawing.Point(572, 165);
            this.label13.Name = "label13";
            this.label13.Size = new System.Drawing.Size(75, 13);
            this.label13.TabIndex = 29;
            this.label13.Text = "(1 E-6 A^-2)";
            // 
            // label14
            // 
            this.label14.AutoSize = true;
            this.label14.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label14.Location = new System.Drawing.Point(540, 165);
            this.label14.Name = "label14";
            this.label14.Size = new System.Drawing.Size(31, 13);
            this.label14.TabIndex = 28;
            this.label14.Text = "SLD";
            // 
            // SLDTB
            // 
            this.SLDTB.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.SLDTB.ForeColor = System.Drawing.SystemColors.WindowText;
            this.SLDTB.Location = new System.Drawing.Point(543, 187);
            this.SLDTB.Name = "SLDTB";
            this.SLDTB.ReadOnly = true;
            this.SLDTB.Size = new System.Drawing.Size(89, 20);
            this.SLDTB.TabIndex = 27;
            // 
            // UseDensCB
            // 
            this.UseDensCB.AutoSize = true;
            this.UseDensCB.Checked = true;
            this.UseDensCB.CheckState = System.Windows.Forms.CheckState.Checked;
            this.UseDensCB.Location = new System.Drawing.Point(22, 343);
            this.UseDensCB.Name = "UseDensCB";
            this.UseDensCB.Size = new System.Drawing.Size(107, 17);
            this.UseDensCB.TabIndex = 30;
            this.UseDensCB.Text = "Use Bulk Density";
            this.UseDensCB.UseVisualStyleBackColor = true;
            this.UseDensCB.CheckedChanged += new System.EventHandler(this.UseDensCB_CheckedChanged);
            // 
            // CalcSLDBB
            // 
            this.CalcSLDBB.Location = new System.Drawing.Point(522, 383);
            this.CalcSLDBB.Name = "CalcSLDBB";
            this.CalcSLDBB.Size = new System.Drawing.Size(127, 40);
            this.CalcSLDBB.TabIndex = 31;
            this.CalcSLDBB.Text = "Calculate";
            this.CalcSLDBB.UseVisualStyleBackColor = true;
            this.CalcSLDBB.Click += new System.EventHandler(this.CalcSLDBB_Click);
            // 
            // NeutronCB
            // 
            this.NeutronCB.AutoSize = true;
            this.NeutronCB.Location = new System.Drawing.Point(518, 353);
            this.NeutronCB.Name = "NeutronCB";
            this.NeutronCB.Size = new System.Drawing.Size(131, 17);
            this.NeutronCB.TabIndex = 32;
            this.NeutronCB.Text = "Calculate for Neutrons";
            this.NeutronCB.UseVisualStyleBackColor = true;
            this.NeutronCB.CheckedChanged += new System.EventHandler(this.NeutronCB_CheckedChanged);
            // 
            // panel1
            // 
            this.panel1.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.panel1.Controls.Add(this.label16);
            this.panel1.Controls.Add(this.SiTB);
            this.panel1.Controls.Add(this.label3);
            this.panel1.Controls.Add(this.ElecTB);
            this.panel1.Controls.Add(this.label2);
            this.panel1.Controls.Add(this.UseElemCB);
            this.panel1.Controls.Add(this.label8);
            this.panel1.Controls.Add(this.DsTB);
            this.panel1.Controls.Add(this.label7);
            this.panel1.Controls.Add(this.CsTB);
            this.panel1.Controls.Add(this.label6);
            this.panel1.Controls.Add(this.NsTB);
            this.panel1.Controls.Add(this.label5);
            this.panel1.Controls.Add(this.OsTB);
            this.panel1.Controls.Add(this.label4);
            this.panel1.Controls.Add(this.PsTB);
            this.panel1.Controls.Add(this.label1);
            this.panel1.Controls.Add(this.HsTB);
            this.panel1.Location = new System.Drawing.Point(8, 9);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(321, 413);
            this.panel1.TabIndex = 33;
            // 
            // panel2
            // 
            this.panel2.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.panel2.Controls.Add(this.label15);
            this.panel2.Controls.Add(this.UseDensCB);
            this.panel2.Controls.Add(this.label12);
            this.panel2.Controls.Add(this.label9);
            this.panel2.Controls.Add(this.ApmTB);
            this.panel2.Controls.Add(this.label10);
            this.panel2.Controls.Add(this.ThickTB);
            this.panel2.Controls.Add(this.label11);
            this.panel2.Controls.Add(this.DensTB);
            this.panel2.Location = new System.Drawing.Point(338, 9);
            this.panel2.Name = "panel2";
            this.panel2.Size = new System.Drawing.Size(149, 413);
            this.panel2.TabIndex = 34;
            // 
            // label15
            // 
            this.label15.AutoSize = true;
            this.label15.Location = new System.Drawing.Point(50, 135);
            this.label15.Name = "label15";
            this.label15.Size = new System.Drawing.Size(18, 13);
            this.label15.TabIndex = 27;
            this.label15.Text = "Or";
            // 
            // label16
            // 
            this.label16.AutoSize = true;
            this.label16.Location = new System.Drawing.Point(26, 310);
            this.label16.Name = "label16";
            this.label16.Size = new System.Drawing.Size(38, 13);
            this.label16.TabIndex = 21;
            this.label16.Text = "Silicon";
            // 
            // SiTB
            // 
            this.SiTB.Location = new System.Drawing.Point(29, 326);
            this.SiTB.Name = "SiTB";
            this.SiTB.Size = new System.Drawing.Size(89, 20);
            this.SiTB.TabIndex = 20;
            this.SiTB.Text = "0";
            this.SiTB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateIntegerInput);
            // 
            // SLDCalculator
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(675, 435);
            this.Controls.Add(this.NeutronCB);
            this.Controls.Add(this.panel2);
            this.Controls.Add(this.panel1);
            this.Controls.Add(this.CalcSLDBB);
            this.Controls.Add(this.label13);
            this.Controls.Add(this.label14);
            this.Controls.Add(this.SLDTB);
            this.Name = "SLDCalculator";
            this.Text = "SLDCalculator";
            this.panel1.ResumeLayout(false);
            this.panel1.PerformLayout();
            this.panel2.ResumeLayout(false);
            this.panel2.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox HsTB;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.TextBox PsTB;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.TextBox OsTB;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.TextBox NsTB;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.TextBox CsTB;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.TextBox DsTB;
        private System.Windows.Forms.CheckBox UseElemCB;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox ElecTB;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.TextBox ApmTB;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.TextBox ThickTB;
        private System.Windows.Forms.Label label11;
        private System.Windows.Forms.TextBox DensTB;
        private System.Windows.Forms.Label label12;
        private System.Windows.Forms.Label label13;
        private System.Windows.Forms.Label label14;
        private System.Windows.Forms.TextBox SLDTB;
        private System.Windows.Forms.CheckBox UseDensCB;
        private System.Windows.Forms.Button CalcSLDBB;
        private System.Windows.Forms.CheckBox NeutronCB;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.Panel panel2;
        private System.Windows.Forms.Label label15;
        private System.Windows.Forms.Label label16;
        private System.Windows.Forms.TextBox SiTB;
    }
}