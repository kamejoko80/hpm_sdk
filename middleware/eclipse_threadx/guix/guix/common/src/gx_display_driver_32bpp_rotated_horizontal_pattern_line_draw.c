/***************************************************************************
 * Copyright (c) 2024 Microsoft Corporation 
 * 
 * This program and the accompanying materials are made available under the
 * terms of the MIT License which is available at
 * https://opensource.org/licenses/MIT.
 * 
 * SPDX-License-Identifier: MIT
 **************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** GUIX Component                                                        */
/**                                                                       */
/**   Display Management (Display)                                        */
/**                                                                       */
/**************************************************************************/
#define GX_SOURCE_CODE

/* Include necessary system files.  */

#include "gx_api.h"
#include "gx_display.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _gx_display_driver_32bpp_roizontal_horizontal_pattern_line_draw     */
/*                                                                        */
/*                                                        PORTABLE C      */
/*                                                           6.1.4        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Kenneth Maxwell, Microsoft Corporation                              */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    Generic 32bpp color format horizontal pattern line draw function.   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    context                               Drawing context               */
/*    xstart                                x-coord of left endpoint      */
/*    xend                                  x-coord of right endpoint     */
/*    ypos                                  y-coord of line top           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    GUIX Internal Code                                                  */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  02-02-2021     Kenneth Maxwell          Initial Version 6.1.4         */
/*                                                                        */
/**************************************************************************/
VOID _gx_display_driver_32bpp_rotated_horizontal_pattern_line_draw(GX_DRAW_CONTEXT *context, INT xstart, INT xend, INT ypos)
{
INT    column;
ULONG *put;
ULONG *rowstart;
ULONG  pattern;
ULONG  mask;
ULONG  on_color;
ULONG  off_color;
INT    len = xend - xstart + 1;

    /* Pick up start address of canvas memory.  */
    rowstart = (ULONG *)context -> gx_draw_context_memory;

    if (context -> gx_draw_context_display -> gx_display_rotation_angle == GX_SCREEN_ROTATION_CW)
    {
        /* Calculate start of row address.  */
        rowstart += (context -> gx_draw_context_canvas -> gx_canvas_x_resolution - xstart - 1) * context -> gx_draw_context_pitch;

        /* Calculate pixel address.  */
        rowstart += ypos;
    }
    else
    {
        /* Calculate start of row address.  */
        rowstart += xend * context -> gx_draw_context_pitch;

        /* Calculate pixel address.  */
        rowstart += (context -> gx_draw_context_canvas -> gx_canvas_y_resolution - ypos - 1);
    }

    /* Pick up the requested pattern and mask.  */
    pattern = context -> gx_draw_context_brush.gx_brush_line_pattern;
    mask = context -> gx_draw_context_brush.gx_brush_pattern_mask;
    on_color = (ULONG)context -> gx_draw_context_brush.gx_brush_line_color;
    off_color = (ULONG)context -> gx_draw_context_brush.gx_brush_fill_color;

    put = rowstart;

    /* Draw one line, left to right.  */
    for (column = 0; column < len; column++)
    {
        if (pattern & mask)
        {
            *put = on_color;
        }
        else
        {
            *put = off_color;
        }

        put -= context -> gx_draw_context_pitch;
        mask >>= 1;
        if (!mask)
        {
            mask = 0x80000000;
        }
    }

    /* Save current masks value back to brush.  */
    context -> gx_draw_context_brush.gx_brush_pattern_mask = mask;
}

