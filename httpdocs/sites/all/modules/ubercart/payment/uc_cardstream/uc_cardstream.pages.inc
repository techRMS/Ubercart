<?
	function uc_cardstream_completed(){
		if($_SESSION['cardstreamoutput']){
			$output = $_SESSION['cardstreamoutput'];
			$title = $_SESSION['cardstreamoutputtitle'];
			$_SESSION['cardstreamoutput'] = "";
			$_SESSION['cardstreamoutputtitle'] = "";
			drupal_set_title($title);
			return($output);
		}else{
			header('Location: /');
		}
	}

	function uc_cardstream_complete($cart_id = 0) {
		$_SESSION['cardstreamoutput'] = "";

		watchdog('cardstream', 'Receiving new order notification for order !order_id.', array('!order_id' => check_plain($_POST['orderRef'])));

		$order = uc_order_load($_POST['orderRef']);

		$formatcheck = strstr($order->order_total, '.');

		if($formatcheck !== false){
			$ordertotal = $order->order_total;
		}else{
			$ordertotal = $order->order_total."00";
		}

		if ($order === FALSE || uc_order_status_data($order->order_status, 'state') != 'in_checkout') {

			uc_order_save($order);
			$_SESSION['cardstreamoutputtitle'] = "Order Failed";
			$_SESSION['cardstreamoutput'] = '<br />Unfortunately your payment wasn\'t successful.<br /><a href="/cart/checkout">Please try again.</a>';

		}elseif((!$_POST['amountReceived']) || (!is_numeric($_POST['amountReceived'])) || (str_replace(".","", $ordertotal) != $_POST['amountReceived']) || ($_POST['responseCode'] != 0)){

			uc_order_save($order);
			$_SESSION['cardstreamoutputtitle'] = "Order Failed";
			$_SESSION['cardstreamoutput'] = '<br />Unfortunately your payment wasn\'t successful.<br /><a href="/cart/checkout">Please try again.</a>';


		}else{
			$usefullinfo = "Message: ".$_POST['responseMessage']." - CrossRef: ".$_POST['xref'];
			uc_payment_enter($order->order_id, 'cardstream', substr($_POST['amountReceived'],0, strlen($_POST['amountReceived'])-2).".".substr($_POST['amountReceived'],strlen($_POST['amountReceived'])-2) , $order->uid, NULL, $usefullinfo);
			uc_order_update_status($order->order_id,"payment_received");

			uc_cart_empty($cart_id);
			$_SESSION['cardstreamoutputtitle'] = "Order Completed";
			$_SESSION['cardstreamoutput'] = uc_cart_complete_sale($order, variable_get('uc_new_customer_login', FALSE));

		}
		//die(var_dump($order,$_POST));
		$page = "cart/cardstream/completed";

		if (!empty($page)) {
			drupal_goto($page);
		}

		return $output;
	}