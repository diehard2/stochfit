namespace StochasticModeling
{
    partial class LevmarOutput
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
            this.LevMarOut = new System.Windows.Forms.TextBox();
            this.SuspendLayout();
            // 
            // LevMarOut
            // 
            this.LevMarOut.BackColor = System.Drawing.SystemColors.Window;
            this.LevMarOut.Cursor = System.Windows.Forms.Cursors.Default;
            this.LevMarOut.ImeMode = System.Windows.Forms.ImeMode.On;
            this.LevMarOut.Location = new System.Drawing.Point(1, 2);
            this.LevMarOut.Multiline = true;
            this.LevMarOut.Name = "LevMarOut";
            this.LevMarOut.ReadOnly = true;
            this.LevMarOut.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.LevMarOut.Size = new System.Drawing.Size(291, 271);
            this.LevMarOut.TabIndex = 0;
            this.LevMarOut.TabStop = false;
            // 
            // LevmarOutput
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(96F, 96F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Dpi;
            this.ClientSize = new System.Drawing.Size(292, 273);
            this.Controls.Add(this.LevMarOut);
            this.Cursor = System.Windows.Forms.Cursors.Default;
            this.DoubleBuffered = true;
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.Fixed3D;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "LevmarOutput";
            this.Text = "Fitting Output";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox LevMarOut;

    }
}