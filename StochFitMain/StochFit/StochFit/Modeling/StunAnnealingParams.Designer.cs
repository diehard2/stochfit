namespace StochasticModeling.Modeling
{
    partial class STUNAnnealingParams
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(STUNAnnealingParams));
            this.tempTB = new System.Windows.Forms.TextBox();
            this.plattb = new System.Windows.Forms.TextBox();
            this.slopeTB = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.GammaTB = new System.Windows.Forms.TextBox();
            this.OK = new System.Windows.Forms.Button();
            this.adaptiveCB = new System.Windows.Forms.CheckBox();
            this.Tempiterlabel = new System.Windows.Forms.Label();
            this.TempIterTB = new System.Windows.Forms.TextBox();
            this.funcCombo = new System.Windows.Forms.ComboBox();
            this.label5 = new System.Windows.Forms.Label();
            this.gammadecTB = new System.Windows.Forms.TextBox();
            this.label6 = new System.Windows.Forms.Label();
            this.STUNdecTB = new System.Windows.Forms.TextBox();
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            this.SuspendLayout();
            // 
            // tempTB
            // 
            this.tempTB.Location = new System.Drawing.Point(25, 33);
            this.tempTB.Name = "tempTB";
            this.tempTB.Size = new System.Drawing.Size(100, 20);
            this.tempTB.TabIndex = 0;
            this.tempTB.Text = "10";
            this.toolTip1.SetToolTip(this.tempTB, "The initial temperature, or the average STUN value if adaptive is selected");
            // 
            // plattb
            // 
            this.plattb.Location = new System.Drawing.Point(25, 92);
            this.plattb.Name = "plattb";
            this.plattb.Size = new System.Drawing.Size(100, 20);
            this.plattb.TabIndex = 1;
            this.plattb.Text = "4000";
            this.toolTip1.SetToolTip(this.plattb, resources.GetString("plattb.ToolTip"));
            // 
            // slopeTB
            // 
            this.slopeTB.Location = new System.Drawing.Point(25, 151);
            this.slopeTB.Name = "slopeTB";
            this.slopeTB.Size = new System.Drawing.Size(100, 20);
            this.slopeTB.TabIndex = 2;
            this.slopeTB.Text = ".95";
            this.toolTip1.SetToolTip(this.slopeTB, "This determines the decrease in the temperature after the selected number of iter" +
                    "ations");
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(22, 17);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(90, 13);
            this.label1.TabIndex = 3;
            this.label1.Text = "Initial temperature";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(22, 135);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(97, 13);
            this.label2.TabIndex = 4;
            this.label2.Text = "Temperature Slope";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(22, 76);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(88, 13);
            this.label3.TabIndex = 5;
            this.label3.Text = "Plateau iterations";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(22, 194);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(88, 13);
            this.label4.TabIndex = 7;
            this.label4.Text = "STUN Parameter";
            // 
            // GammaTB
            // 
            this.GammaTB.Location = new System.Drawing.Point(25, 210);
            this.GammaTB.Name = "GammaTB";
            this.GammaTB.Size = new System.Drawing.Size(100, 20);
            this.GammaTB.TabIndex = 6;
            this.GammaTB.Text = ".05";
            this.toolTip1.SetToolTip(this.GammaTB, "This is gamma for the STUN function");
            // 
            // OK
            // 
            this.OK.Location = new System.Drawing.Point(139, 311);
            this.OK.Name = "OK";
            this.OK.Size = new System.Drawing.Size(89, 28);
            this.OK.TabIndex = 8;
            this.OK.Text = "OK";
            this.OK.UseVisualStyleBackColor = true;
            this.OK.Click += new System.EventHandler(this.OK_Click);
            // 
            // adaptiveCB
            // 
            this.adaptiveCB.AutoSize = true;
            this.adaptiveCB.Location = new System.Drawing.Point(180, 33);
            this.adaptiveCB.Name = "adaptiveCB";
            this.adaptiveCB.Size = new System.Drawing.Size(68, 17);
            this.adaptiveCB.TabIndex = 9;
            this.adaptiveCB.Text = "Adaptive";
            this.adaptiveCB.UseVisualStyleBackColor = true;
            this.adaptiveCB.CheckedChanged += new System.EventHandler(this.adaptiveCB_CheckedChanged);
            // 
            // Tempiterlabel
            // 
            this.Tempiterlabel.AutoSize = true;
            this.Tempiterlabel.Location = new System.Drawing.Point(177, 135);
            this.Tempiterlabel.Name = "Tempiterlabel";
            this.Tempiterlabel.Size = new System.Drawing.Size(151, 13);
            this.Tempiterlabel.TabIndex = 12;
            this.Tempiterlabel.Text = "Temperature change iterations";
            this.Tempiterlabel.Visible = false;
            // 
            // TempIterTB
            // 
            this.TempIterTB.Location = new System.Drawing.Point(180, 151);
            this.TempIterTB.Name = "TempIterTB";
            this.TempIterTB.Size = new System.Drawing.Size(100, 20);
            this.TempIterTB.TabIndex = 11;
            this.TempIterTB.Text = "100";
            this.toolTip1.SetToolTip(this.TempIterTB, "This determines how many iterations are performed before the temperature is chang" +
                    "ed");
            this.TempIterTB.Visible = false;
            // 
            // funcCombo
            // 
            this.funcCombo.FormattingEnabled = true;
            this.funcCombo.Items.AddRange(new object[] {
            "STUN",
            "SINH",
            "LN"});
            this.funcCombo.Location = new System.Drawing.Point(25, 253);
            this.funcCombo.Name = "funcCombo";
            this.funcCombo.Size = new System.Drawing.Size(131, 21);
            this.funcCombo.TabIndex = 14;
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(177, 76);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(134, 13);
            this.label5.TabIndex = 16;
            this.label5.Text = "STUN parameter decrease";
            this.label5.Visible = false;
            // 
            // gammadecTB
            // 
            this.gammadecTB.Location = new System.Drawing.Point(180, 92);
            this.gammadecTB.Name = "gammadecTB";
            this.gammadecTB.Size = new System.Drawing.Size(100, 20);
            this.gammadecTB.TabIndex = 15;
            this.gammadecTB.Text = "0.85";
            this.toolTip1.SetToolTip(this.gammadecTB, "The percentage by which the average STUN value is decreased after \"STUN decrease " +
                    "iterations\"");
            this.gammadecTB.Visible = false;
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(177, 194);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(129, 13);
            this.label6.TabIndex = 18;
            this.label6.Text = "STUN decrease iterations";
            this.label6.Visible = false;
            // 
            // STUNdecTB
            // 
            this.STUNdecTB.Location = new System.Drawing.Point(180, 210);
            this.STUNdecTB.Name = "STUNdecTB";
            this.STUNdecTB.Size = new System.Drawing.Size(100, 20);
            this.STUNdecTB.TabIndex = 17;
            this.STUNdecTB.Text = "200000";
            this.toolTip1.SetToolTip(this.STUNdecTB, "The number of iterations before the Average STUN value decreases by the STUN para" +
                    "meter decrease field");
            this.STUNdecTB.Visible = false;
            // 
            // STUNAnnealingParams
            // 
            this.AcceptButton = this.OK;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(348, 361);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.STUNdecTB);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.gammadecTB);
            this.Controls.Add(this.funcCombo);
            this.Controls.Add(this.Tempiterlabel);
            this.Controls.Add(this.TempIterTB);
            this.Controls.Add(this.adaptiveCB);
            this.Controls.Add(this.OK);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.GammaTB);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.slopeTB);
            this.Controls.Add(this.plattb);
            this.Controls.Add(this.tempTB);
            this.Name = "STUNAnnealingParams";
            this.Text = "Annealing Parameters";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox tempTB;
        private System.Windows.Forms.TextBox plattb;
        private System.Windows.Forms.TextBox slopeTB;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.TextBox GammaTB;
        private System.Windows.Forms.Button OK;
        private System.Windows.Forms.CheckBox adaptiveCB;
        private System.Windows.Forms.Label Tempiterlabel;
        private System.Windows.Forms.TextBox TempIterTB;
        private System.Windows.Forms.ComboBox funcCombo;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.TextBox gammadecTB;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.TextBox STUNdecTB;
        private System.Windows.Forms.ToolTip toolTip1;
    }
}