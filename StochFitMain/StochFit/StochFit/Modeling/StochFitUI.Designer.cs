namespace StochasticModeling.Modeling
{
    partial class StochFitUI
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(StochFitUI));
            this.OK = new System.Windows.Forms.Button();
            this.Cancel = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.description = new System.Windows.Forms.TextBox();
            this.IterTB = new System.Windows.Forms.TextBox();
            this.EDHighTB = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.LengthHighTB = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.RoughHighTB = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.ErrorCutTB = new System.Windows.Forms.TextBox();
            this.ErrCutCB = new System.Windows.Forms.CheckBox();
            this.RoughLowTB = new System.Windows.Forms.TextBox();
            this.LengthLowTB = new System.Windows.Forms.TextBox();
            this.EDLowTB = new System.Windows.Forms.TextBox();
            this.label6 = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.label8 = new System.Windows.Forms.Label();
            this.label9 = new System.Windows.Forms.Label();
            this.label10 = new System.Windows.Forms.Label();
            this.label11 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // OK
            // 
            this.OK.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.OK.Location = new System.Drawing.Point(240, 302);
            this.OK.Name = "OK";
            this.OK.Size = new System.Drawing.Size(76, 29);
            this.OK.TabIndex = 9;
            this.OK.Text = "OK";
            this.OK.UseVisualStyleBackColor = true;
            // 
            // Cancel
            // 
            this.Cancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.Cancel.Location = new System.Drawing.Point(334, 302);
            this.Cancel.Name = "Cancel";
            this.Cancel.Size = new System.Drawing.Size(76, 29);
            this.Cancel.TabIndex = 10;
            this.Cancel.Text = "Cancel";
            this.Cancel.UseVisualStyleBackColor = true;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(29, 13);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(103, 13);
            this.label1.TabIndex = 2;
            this.label1.Text = "Stochastic Iterations";
            // 
            // description
            // 
            this.description.Enabled = false;
            this.description.Location = new System.Drawing.Point(240, 88);
            this.description.Multiline = true;
            this.description.Name = "description";
            this.description.ReadOnly = true;
            this.description.Size = new System.Drawing.Size(201, 141);
            this.description.TabIndex = 3;
            this.description.Text = resources.GetString("description.Text");
            // 
            // IterTB
            // 
            this.IterTB.Location = new System.Drawing.Point(29, 29);
            this.IterTB.Name = "IterTB";
            this.IterTB.Size = new System.Drawing.Size(111, 20);
            this.IterTB.TabIndex = 0;
            this.IterTB.Text = "10000";
            this.IterTB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateIntegerInput);
            // 
            // EDHighTB
            // 
            this.EDHighTB.Location = new System.Drawing.Point(43, 107);
            this.EDHighTB.Name = "EDHighTB";
            this.EDHighTB.Size = new System.Drawing.Size(61, 20);
            this.EDHighTB.TabIndex = 1;
            this.EDHighTB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateIntegerInput);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(29, 69);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(142, 13);
            this.label2.TabIndex = 5;
            this.label2.Text = "Electron Density Percentage";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(29, 141);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(143, 13);
            this.label3.TabIndex = 7;
            this.label3.Text = "Layer Thickness Percentage";
            // 
            // LengthHighTB
            // 
            this.LengthHighTB.Location = new System.Drawing.Point(43, 180);
            this.LengthHighTB.Name = "LengthHighTB";
            this.LengthHighTB.Size = new System.Drawing.Size(61, 20);
            this.LengthHighTB.TabIndex = 3;
            this.LengthHighTB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateIntegerInput);
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(29, 216);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(119, 13);
            this.label4.TabIndex = 9;
            this.label4.Text = "Roughness Percentage";
            // 
            // RoughHighTB
            // 
            this.RoughHighTB.Location = new System.Drawing.Point(43, 255);
            this.RoughHighTB.Name = "RoughHighTB";
            this.RoughHighTB.Size = new System.Drawing.Size(61, 20);
            this.RoughHighTB.TabIndex = 5;
            this.RoughHighTB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateIntegerInput);
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(40, 331);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(118, 13);
            this.label5.TabIndex = 11;
            this.label5.Text = "Error Cutoff Percentage";
            // 
            // ErrorCutTB
            // 
            this.ErrorCutTB.Location = new System.Drawing.Point(40, 350);
            this.ErrorCutTB.Name = "ErrorCutTB";
            this.ErrorCutTB.Size = new System.Drawing.Size(100, 20);
            this.ErrorCutTB.TabIndex = 8;
            this.ErrorCutTB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // ErrCutCB
            // 
            this.ErrCutCB.AutoSize = true;
            this.ErrCutCB.Checked = true;
            this.ErrCutCB.CheckState = System.Windows.Forms.CheckState.Checked;
            this.ErrCutCB.Location = new System.Drawing.Point(29, 302);
            this.ErrCutCB.Name = "ErrCutCB";
            this.ErrCutCB.Size = new System.Drawing.Size(101, 17);
            this.ErrCutCB.TabIndex = 7;
            this.ErrCutCB.Text = "Use Error Cutoff";
            this.ErrCutCB.UseVisualStyleBackColor = true;
            this.ErrCutCB.CheckedChanged += new System.EventHandler(this.ErrCutCB_CheckedChanged);
            // 
            // RoughLowTB
            // 
            this.RoughLowTB.Location = new System.Drawing.Point(136, 255);
            this.RoughLowTB.Name = "RoughLowTB";
            this.RoughLowTB.Size = new System.Drawing.Size(61, 20);
            this.RoughLowTB.TabIndex = 6;
            this.RoughLowTB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateIntegerInput);
            // 
            // LengthLowTB
            // 
            this.LengthLowTB.Location = new System.Drawing.Point(136, 180);
            this.LengthLowTB.Name = "LengthLowTB";
            this.LengthLowTB.Size = new System.Drawing.Size(61, 20);
            this.LengthLowTB.TabIndex = 4;
            this.LengthLowTB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateIntegerInput);
            // 
            // EDLowTB
            // 
            this.EDLowTB.Location = new System.Drawing.Point(136, 107);
            this.EDLowTB.Name = "EDLowTB";
            this.EDLowTB.Size = new System.Drawing.Size(61, 20);
            this.EDLowTB.TabIndex = 2;
            this.EDLowTB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateIntegerInput);
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(40, 91);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(29, 13);
            this.label6.TabIndex = 16;
            this.label6.Text = "High";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(133, 91);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(27, 13);
            this.label7.TabIndex = 17;
            this.label7.Text = "Low";
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(133, 239);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(27, 13);
            this.label8.TabIndex = 19;
            this.label8.Text = "Low";
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(40, 239);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(29, 13);
            this.label9.TabIndex = 18;
            this.label9.Text = "High";
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Location = new System.Drawing.Point(133, 164);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(27, 13);
            this.label10.TabIndex = 21;
            this.label10.Text = "Low";
            // 
            // label11
            // 
            this.label11.AutoSize = true;
            this.label11.Location = new System.Drawing.Point(40, 164);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(29, 13);
            this.label11.TabIndex = 20;
            this.label11.Text = "High";
            // 
            // StochFitUI
            // 
            this.AcceptButton = this.OK;
            this.AutoScaleDimensions = new System.Drawing.SizeF(96F, 96F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Dpi;
            this.CancelButton = this.Cancel;
            this.ClientSize = new System.Drawing.Size(453, 393);
            this.Controls.Add(this.label10);
            this.Controls.Add(this.label11);
            this.Controls.Add(this.label8);
            this.Controls.Add(this.label9);
            this.Controls.Add(this.label7);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.EDLowTB);
            this.Controls.Add(this.LengthLowTB);
            this.Controls.Add(this.RoughLowTB);
            this.Controls.Add(this.ErrCutCB);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.ErrorCutTB);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.RoughHighTB);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.LengthHighTB);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.EDHighTB);
            this.Controls.Add(this.IterTB);
            this.Controls.Add(this.description);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.Cancel);
            this.Controls.Add(this.OK);
            this.Name = "StochFitUI";
            this.Text = "Stochastic Search Options";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button OK;
        private System.Windows.Forms.Button Cancel;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox description;
        private System.Windows.Forms.TextBox IterTB;
        private System.Windows.Forms.TextBox EDHighTB;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox LengthHighTB;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.TextBox RoughHighTB;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.TextBox ErrorCutTB;
        private System.Windows.Forms.CheckBox ErrCutCB;
        private System.Windows.Forms.TextBox RoughLowTB;
        private System.Windows.Forms.TextBox LengthLowTB;
        private System.Windows.Forms.TextBox EDLowTB;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.Label label11;
    }
}