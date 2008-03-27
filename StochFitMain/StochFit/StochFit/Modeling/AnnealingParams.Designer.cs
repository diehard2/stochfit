namespace StochasticModeling.Modeling
{
    partial class AnnealingParams
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
            this.tempTB = new System.Windows.Forms.TextBox();
            this.plattb = new System.Windows.Forms.TextBox();
            this.slopeTB = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.OK = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // tempTB
            // 
            this.tempTB.Location = new System.Drawing.Point(25, 33);
            this.tempTB.Name = "tempTB";
            this.tempTB.Size = new System.Drawing.Size(100, 20);
            this.tempTB.TabIndex = 0;
            this.tempTB.Text = "10";
            this.tempTB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
            // 
            // plattb
            // 
            this.plattb.Location = new System.Drawing.Point(25, 92);
            this.plattb.Name = "plattb";
            this.plattb.Size = new System.Drawing.Size(100, 20);
            this.plattb.TabIndex = 1;
            this.plattb.Text = "4000";
            this.plattb.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateIntegerInput);
            // 
            // slopeTB
            // 
            this.slopeTB.Location = new System.Drawing.Point(25, 151);
            this.slopeTB.Name = "slopeTB";
            this.slopeTB.Size = new System.Drawing.Size(100, 20);
            this.slopeTB.TabIndex = 2;
            this.slopeTB.Text = ".95";
            this.slopeTB.Validating += new System.ComponentModel.CancelEventHandler(this.ValidateNumericalInput);
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
            // OK
            // 
            this.OK.Location = new System.Drawing.Point(191, 143);
            this.OK.Name = "OK";
            this.OK.Size = new System.Drawing.Size(89, 28);
            this.OK.TabIndex = 8;
            this.OK.Text = "OK";
            this.OK.UseVisualStyleBackColor = true;
            this.OK.Click += new System.EventHandler(this.OK_Click);
            // 
            // AnnealingParams
            // 
            this.AcceptButton = this.OK;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(292, 201);
            this.Controls.Add(this.OK);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.slopeTB);
            this.Controls.Add(this.plattb);
            this.Controls.Add(this.tempTB);
            this.Name = "AnnealingParams";
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
        private System.Windows.Forms.Button OK;
    }
}