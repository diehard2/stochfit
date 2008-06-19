namespace StochasticModeling.GraphingZoom
{
    partial class AdvancedZoom
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
            this.xlowerTB = new System.Windows.Forms.TextBox();
            this.xupperTB = new System.Windows.Forms.TextBox();
            this.ylowerTB = new System.Windows.Forms.TextBox();
            this.yupperTB = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.button1 = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // xlowerTB
            // 
            this.xlowerTB.Location = new System.Drawing.Point(106, 28);
            this.xlowerTB.Name = "xlowerTB";
            this.xlowerTB.Size = new System.Drawing.Size(90, 20);
            this.xlowerTB.TabIndex = 0;
            // 
            // xupperTB
            // 
            this.xupperTB.Location = new System.Drawing.Point(106, 70);
            this.xupperTB.Name = "xupperTB";
            this.xupperTB.Size = new System.Drawing.Size(90, 20);
            this.xupperTB.TabIndex = 1;
            // 
            // ylowerTB
            // 
            this.ylowerTB.Location = new System.Drawing.Point(106, 116);
            this.ylowerTB.Name = "ylowerTB";
            this.ylowerTB.Size = new System.Drawing.Size(90, 20);
            this.ylowerTB.TabIndex = 2;
            // 
            // yupperTB
            // 
            this.yupperTB.Location = new System.Drawing.Point(106, 161);
            this.yupperTB.Name = "yupperTB";
            this.yupperTB.Size = new System.Drawing.Size(90, 20);
            this.yupperTB.TabIndex = 3;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(103, 12);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(62, 13);
            this.label1.TabIndex = 4;
            this.label1.Text = "X lower limit";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(103, 54);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(64, 13);
            this.label2.TabIndex = 5;
            this.label2.Text = "X upper limit";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(103, 100);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(62, 13);
            this.label3.TabIndex = 6;
            this.label3.Text = "Y lower limit";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(103, 139);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(64, 13);
            this.label4.TabIndex = 7;
            this.label4.Text = "Y upper limit";
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(106, 207);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(89, 36);
            this.button1.TabIndex = 8;
            this.button1.Text = "OK";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // AdvancedZoom
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(284, 264);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.yupperTB);
            this.Controls.Add(this.ylowerTB);
            this.Controls.Add(this.xupperTB);
            this.Controls.Add(this.xlowerTB);
            this.Name = "AdvancedZoom";
            this.Text = "Set Advance Zoom";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox xlowerTB;
        private System.Windows.Forms.TextBox xupperTB;
        private System.Windows.Forms.TextBox ylowerTB;
        private System.Windows.Forms.TextBox yupperTB;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Button button1;
    }
}