namespace NURB
{
    partial class Form1
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
            this.SplineGraphControl = new ZedGraph.ZedGraphControl();
            this.Resetbutton = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // SplineGraphControl
            // 
            this.SplineGraphControl.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.SplineGraphControl.AutoSize = true;
            this.SplineGraphControl.EditButtons = System.Windows.Forms.MouseButtons.Left;
            this.SplineGraphControl.Location = new System.Drawing.Point(12, 12);
            this.SplineGraphControl.Name = "SplineGraphControl";
            this.SplineGraphControl.PanModifierKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Shift | System.Windows.Forms.Keys.None)));
            this.SplineGraphControl.ScrollGrace = 0;
            this.SplineGraphControl.ScrollMaxX = 0;
            this.SplineGraphControl.ScrollMaxY = 0;
            this.SplineGraphControl.ScrollMaxY2 = 0;
            this.SplineGraphControl.ScrollMinX = 0;
            this.SplineGraphControl.ScrollMinY = 0;
            this.SplineGraphControl.ScrollMinY2 = 0;
            this.SplineGraphControl.Size = new System.Drawing.Size(485, 374);
            this.SplineGraphControl.TabIndex = 10;
            // 
            // Resetbutton
            // 
            this.Resetbutton.Location = new System.Drawing.Point(523, 327);
            this.Resetbutton.Name = "Resetbutton";
            this.Resetbutton.Size = new System.Drawing.Size(81, 30);
            this.Resetbutton.TabIndex = 11;
            this.Resetbutton.Text = "Reset";
            this.Resetbutton.UseVisualStyleBackColor = true;
            this.Resetbutton.Click += new System.EventHandler(this.Resetbutton_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(620, 398);
            this.Controls.Add(this.Resetbutton);
            this.Controls.Add(this.SplineGraphControl);
            this.Name = "Form1";
            this.Text = "Form1";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private ZedGraph.ZedGraphControl SplineGraphControl;
        private System.Windows.Forms.Button Resetbutton;
    }
}

