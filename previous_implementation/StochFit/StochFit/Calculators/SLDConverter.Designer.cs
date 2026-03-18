namespace StochasticModeling
{
    partial class SLDConverter
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
            this.XRSLD = new System.Windows.Forms.TextBox();
            this.ECount = new System.Windows.Forms.TextBox();
            this.BSUM = new System.Windows.Forms.TextBox();
            this.NSLD = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.button1 = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // XRSLD
            // 
            this.XRSLD.Location = new System.Drawing.Point(28, 110);
            this.XRSLD.Name = "XRSLD";
            this.XRSLD.Size = new System.Drawing.Size(86, 20);
            this.XRSLD.TabIndex = 0;
            // 
            // ECount
            // 
            this.ECount.Location = new System.Drawing.Point(192, 168);
            this.ECount.Name = "ECount";
            this.ECount.Size = new System.Drawing.Size(86, 20);
            this.ECount.TabIndex = 1;
            // 
            // BSUM
            // 
            this.BSUM.Location = new System.Drawing.Point(192, 73);
            this.BSUM.Name = "BSUM";
            this.BSUM.Size = new System.Drawing.Size(86, 20);
            this.BSUM.TabIndex = 2;
            // 
            // NSLD
            // 
            this.NSLD.Location = new System.Drawing.Point(331, 110);
            this.NSLD.Name = "NSLD";
            this.NSLD.Size = new System.Drawing.Size(86, 20);
            this.NSLD.TabIndex = 3;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(25, 93);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(46, 13);
            this.label1.TabIndex = 4;
            this.label1.Text = "XR SLD";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(328, 94);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(69, 13);
            this.label2.TabIndex = 5;
            this.label2.Text = "Neutron SLD";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(189, 152);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(77, 13);
            this.label3.TabIndex = 6;
            this.label3.Text = "Electron Count";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(189, 57);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(116, 13);
            this.label4.TabIndex = 7;
            this.label4.Text = "Bound Incoherent Sum";
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(318, 297);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(99, 33);
            this.button1.TabIndex = 8;
            this.button1.Text = "Calculate";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // SLDConverter
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(471, 356);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.NSLD);
            this.Controls.Add(this.BSUM);
            this.Controls.Add(this.ECount);
            this.Controls.Add(this.XRSLD);
            this.Name = "SLDConverter";
            this.Text = "SLDConverter";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox XRSLD;
        private System.Windows.Forms.TextBox ECount;
        private System.Windows.Forms.TextBox BSUM;
        private System.Windows.Forms.TextBox NSLD;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Button button1;
    }
}