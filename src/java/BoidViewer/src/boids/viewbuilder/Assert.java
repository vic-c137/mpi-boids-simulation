package boids.viewbuilder;

//=============================================================================================================================================================
/**
 * Defines assertion functionality globally throughout the system. It is declared abstract to play a singleton role.
 * 
 * @author Dan Tappan [14.09.12]
 */
public abstract class Assert
{
   // ---------------------------------------------------------------------------------------------------------------------------------------------------------
   /**
    * Validates a list of booleans as being true with an assertion.
    * 
    * @param values - the values
    */
   public static void isTrue(final boolean... values)
   {
      nonnull(values);

      for (int iValue = 0; iValue < values.length; ++iValue)
      {
         boolean value = values[iValue];

         if (!value)
         {
            throw new AssertionError("argument " + (iValue + 1) + " of " + values.length + " false");
         }
      }
   }

   // ---------------------------------------------------------------------------------------------------------------------------------------------------------
   /**
    * Validates a list of objects as being non-null with an assertion.
    * 
    * @param values - the values
    */
   public static void nonnull(final Object... values)
   {
      if (values == null)
      {
         throw new AssertionError("null");
      }

      for (int iValue = 0; iValue < values.length; ++iValue)
      {
         Object value = values[iValue];

         if (value == null)
         {
            throw new AssertionError("argument " + (iValue + 1) + " of " + values.length + " null");
         }
      }
   }

   // ---------------------------------------------------------------------------------------------------------------------------------------------------------
   /**
    * Validates a list of strings as being non-null and non-empty with an assertion.
    * 
    * @param values - the values
    */
   public static void nonnullempty(final String... values)
   {
      if (values == null)
      {
         throw new AssertionError("null");
      }

      for (int iValue = 0; iValue < values.length; ++iValue)
      {
         String value = values[iValue];

         if (value == null)
         {
            throw new AssertionError("argument " + (iValue + 1) + " of " + values.length + " null");
         }

         if (value.isEmpty())
         {
            throw new AssertionError("argument " + (iValue + 1) + " of " + values.length + " empty");
         }
      }
   }

   // ---------------------------------------------------------------------------------------------------------------------------------------------------------
   /**
    * Validates an optional argument as being absent or present only once with an assertion.
    * 
    * @param argument - the argument
    */
   public static void optional(final Object[] argument)
   {
      nonnull(argument);

      if (argument.length > 1)
      {
         throw new AssertionError("zero or one arguments allowed, not " + argument.length);
      }
   }

   // ---------------------------------------------------------------------------------------------------------------------------------------------------------
   /**
    * Validates a list of values as being positive with an assertion.
    * 
    * @param values - the values
    */
   public static void positive(final double... values)
   {
      nonnull(values);

      for (int iValue = 0; iValue < values.length; ++iValue)
      {
         double value = values[iValue];

         if (value <= 0)
         {
            throw new AssertionError("argument " + (iValue + 1) + " of " + values.length + " value " + value);
         }
      }
   }

   // ---------------------------------------------------------------------------------------------------------------------------------------------------------
   /**
    * Validates a numerical value as being on a range.
    * 
    * @param value - the value
    * @param min - the minimum legal value
    * @param max - the maximum legal value
    */
   public static void range(final double value, final double min, final double max)
   {
      if (value < min)
      {
         throw new AssertionError(value + "<" + min);
      }
      if (value > max)
      {
         throw new AssertionError(value + ">" + max);
      }
   }

}
