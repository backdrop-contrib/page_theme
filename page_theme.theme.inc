<?php

/**
 * @file
 * Theme functions for the page_theme module.
 */


/**
 * Returns HTML for the page_theme administration overview form.
 *
 * @param $variables
 *   An associative array containing:
 *   - form: A render element representing the form.
 *
 * @ingroup themeable
 */
function theme_page_theme_admin_overview($variables) {

  $form = $variables['form'];

  $rows = array();

  foreach (element_children($form['rules']) as $rule) {

    $form['rules'][$rule]['weight']['#attributes']['class'] = array('page-theme-order-weight');

    $row = array();
    $row[] = backdrop_render($form['rules'][$rule]['name']);
    $row[] = backdrop_render($form['rules'][$rule]['theme']);
    $row[] = backdrop_render($form['rules'][$rule]['pages']);
    $row[] = backdrop_render($form['rules'][$rule]['roles']);
    $row[] = backdrop_render($form['rules'][$rule]['status']);
    $row[] = backdrop_render($form['rules'][$rule]['weight']);
    $row[] = backdrop_render($form['rules'][$rule]['operations']);

    $rows[] = array('data' => $row, 'class' => array('draggable'));
  }

  $header = array();
  $header[] = t('Name');
  $header[] = t('Theme');
  $header[] = t('Pages');
  $header[] = t('Roles');
  $header[] = t('Enabled');
  $header[] = t('Weight');
  $header[] = t('Operations');

  $output = backdrop_render($form['information']);
  $output .= theme('table', array(
    'header' => $header,
    'rows' => $rows,
    'attributes' => array('id' => 'page-theme-order'),
    'empty' => backdrop_placeholder(t('No rules defined.')),
  ));
  $output .= backdrop_render_children($form);

  backdrop_add_tabledrag('page-theme-order', 'order', 'sibling', 'page-theme-order-weight');

  return $output;
}
